module;

#include <boost/unordered/unordered_flat_map.hpp>
#include <magic_enum/magic_enum.hpp>

module Chess.LegalMoveGeneration:Magic;

import std;
import nlohmann.json;

import Chess.EasyRandom;
import Chess.EnvironmentVariable;
import Chess.Direction;
import Chess.RankCalculator;

import :ChainedMoveGenerator;
import :MoveGen;

namespace chess {
	template<dir::Direction Direction>
	struct SlidingMoveGenerator {
		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const {
			MoveGen ret;

			for (int i = 0; i < 7; i++) {
				movingPieces = Direction::move(movingPieces) & Direction::NON_BORDERS;
				if (!movingPieces) { //if we hit the wraparound border, break
					break;
				}

				ret.emptyDestSquares |= movingPieces & empty;
				ret.nonEmptyDestSquares |= movingPieces & ~empty;

				movingPieces &= empty;
			}
			return ret;
		}
	};

	constexpr SlidingMoveGenerator<dir::sliding::NorthWest> northWestSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::NorthEast> northEastSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::SouthWest> southWestSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::SouthEast> southEastSlidingMoveGenerator;

	constexpr ChainedMoveGenerator diagonalMoveGenerator{
		northWestSlidingMoveGenerator,
		northEastSlidingMoveGenerator,
		southWestSlidingMoveGenerator,
		southEastSlidingMoveGenerator
	};

	constexpr SlidingMoveGenerator<dir::sliding::North> northSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::East> eastSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::South> southSlidingMoveGenerator;
	constexpr SlidingMoveGenerator<dir::sliding::West> westSlidingMoveGenerator;

	constexpr ChainedMoveGenerator orthogonalMoveGenerator{
		northSlidingMoveGenerator,
		eastSlidingMoveGenerator,
		southSlidingMoveGenerator,
		westSlidingMoveGenerator
	};

	Bitboard getMagicIndex(Bitboard pieces, Bitboard magicNum, int squareCount) {
		return (pieces * magicNum) >> (64_bb - squareCount);
	}

	template<typename Storage>
	struct Magic {
		Bitboard rays = 0;
		int raySquareCount = 0;
		Bitboard num = 0;
		Storage possiblePositions;

		Bitboard operator[](Bitboard allPieces) const {
			auto index = getMagicIndex(allPieces & rays, num, raySquareCount);
			return possiblePositions[index];
		}
	};
	using DynamicMagic = Magic<std::vector<Bitboard>>;
	using StaticMagic  = Magic<std::span<const Bitboard>>;

	class AllPositions {
	private:
		static inline std::vector<Bitboard> m_allPositions;
		static inline size_t m_index = 0;
	public:
		static void init(size_t n) {
			m_allPositions.resize(n);
		}
		static std::span<const Bitboard> addPositions(const std::vector<Bitboard>& positions) {
			std::span ret{ m_allPositions.data() + m_index, positions.size() };
			std::ranges::copy(positions, m_allPositions.begin() + m_index);
			m_index += positions.size();
			return ret;
		}
	};

	void from_json(const nlohmann::json& j, StaticMagic& m) {
		m.rays = j["rays"].get<Bitboard>();
		m.raySquareCount = std::popcount(m.rays);
		m.num = j["num"].get<Bitboard>();

		auto possiblePositions = j["possible_positions"].get<std::vector<Bitboard>>();
		m.possiblePositions = AllPositions::addPositions(possiblePositions);
	}

	void to_json(nlohmann::json& j, const DynamicMagic& m) {
		j["rays"] = m.rays;
		j["num"] = m.num;
		j["possible_positions"] = m.possiblePositions;
	}

	class MagicNumberGenerator {
	private:
		static Bitboard ithSubBoard(Bitboard src, Bitboard i) {
			Bitboard subBoard = 0;

			auto s = None;
			while (nextSquare(i, s)) {
				subBoard |= makeBitboard(getNthSetSquare(src, s));
			}
			return subBoard;
		}

		template<std::invocable<Bitboard> Func>
		void forEachSubBoard(Bitboard board, Func f) const {
			auto possiblePositionCount = m_subBoardCount;
			for (Bitboard i = 0; i < possiblePositionCount; i++) {
				auto ithBoard = ithSubBoard(board, i);
				f(ithBoard);
			}
		}

		struct DestinationSquareData {
			Bitboard blockers = 0;
			std::vector<Bitboard> pieceConfigurations;
		};

		boost::unordered_flat_map<Bitboard, DestinationSquareData> m_destSquareData;
		boost::unordered_flat_map<Bitboard, Bitboard> m_indicesToDestSquares;
		int m_raySquareCount = 0; //for rays
		int m_subBoardCount = 0;
	public:
		template<typename MoveGenerator>
		MagicNumberGenerator(Square movedPiece, Bitboard rays, MoveGenerator moveGenerator) {
			m_raySquareCount = std::popcount(rays);
			m_subBoardCount = 1 << m_raySquareCount;

			forEachSubBoard(rays, [&, this](Bitboard pieces) {
				auto destSquares = moveGenerator(makeBitboard(movedPiece), ~pieces);
				auto allDestSquares = destSquares.all();
				auto blockers = destSquares.nonEmptyDestSquares;
				
				auto it = m_destSquareData.find(allDestSquares);
				if (it == m_destSquareData.end()) {
					DestinationSquareData destSquareData{ blockers, std::vector{ pieces } };
					m_destSquareData.emplace(allDestSquares, std::move(destSquareData));
				} else { //redundant movedPiece configuration
					assert(!std::ranges::contains(it->second.pieceConfigurations, pieces));
					assert(it->second.blockers == blockers);
					it->second.pieceConfigurations.push_back(pieces);
				}
			});
		}
	private:
		bool testMagicNumberImpl(Bitboard magicNum, Bitboard allDestSquares, const DestinationSquareData& destSquareData)
		{
			using std::ranges::all_of;
			return all_of(destSquareData.pieceConfigurations, [&, this](Bitboard pieces) {
				auto index = getMagicIndex(pieces, magicNum, m_raySquareCount);
				auto indexIt = m_indicesToDestSquares.find(index);
				if (indexIt != m_indicesToDestSquares.end()) {
					return indexIt->second == allDestSquares;
				}
				m_indicesToDestSquares.emplace(index, allDestSquares);
				return true;
			});
		}
	
		bool isValidMagicNumber(Bitboard magicNum) {
			using std::ranges::all_of;
			return all_of(m_destSquareData, [&](const auto& destSquareData) {
				const auto& [allDestSquares, pieceConfigurations] = destSquareData;
				return testMagicNumberImpl(magicNum, allDestSquares, pieceConfigurations);
			});
		}
	public:
		Bitboard make() {
			constexpr auto BB_MAX = std::numeric_limits<Bitboard>::max();
			while (true) {
				m_indicesToDestSquares.clear();

				auto r1 = makeRandomNum(0_bb, BB_MAX);
				auto r2 = makeRandomNum(0_bb, BB_MAX);
				auto r3 = makeRandomNum(0_bb, BB_MAX);
				auto magicNum = r1 & r2 & r3;
				if (isValidMagicNumber(magicNum)) {
					return magicNum;
				}
			}
			std::unreachable();
			return 0;
		}

		std::vector<Bitboard> getAllPositions() const {
			std::vector<Bitboard> vec;
			vec.resize(static_cast<size_t>(m_subBoardCount));
			for (const auto& [index, destSquares] : m_indicesToDestSquares) {
				vec[index] = destSquares;
			}
			return vec;
		}
	};

	//note: calcFile<N> and calcRank<N> return the n-1th file and rank, respectively
	Bitboard trimEdges(Square movedPiece, Bitboard otherPieces) {
		auto file = fileOf(movedPiece);
		if (file != 0) {
			otherPieces &= ~calcFile<1>();
		}
		if (file != 7) {
			otherPieces &= ~calcFile<8>(); 
		}

		auto rank = rankOf(movedPiece);
		if (rank != 0) {
			otherPieces &= ~calcRank<1>();
		}
		if (rank != 7) {
			otherPieces &= ~calcRank<8>();
		}
		return otherPieces;
	}

	template<typename MoveGenerator>
	void mapSquare(SquareMap<DynamicMagic>& map, Square square, MoveGenerator moveGenerator) {
		auto pieceBoard = makeBitboard(square);
		auto empty = ALL_SQUARES & ~pieceBoard;
		auto rays = moveGenerator(pieceBoard, empty).all();
		rays = trimEdges(square, rays);

		MagicNumberGenerator generator{ square, rays, moveGenerator };
		auto magicNum = generator.make();
		auto positions = generator.getAllPositions();
		
		map[square] = DynamicMagic{ rays, std::popcount(rays), magicNum, positions };
	}

	template<typename Magic>
	struct MagicMaps {
		SquareMap<Magic> orthogonalMoveMap;
		SquareMap<Magic> diagonalMoveMap;
	};

	std::filesystem::path getTablePath() {
		return getAssetDirectoryPath() / "magic_table.json";
	}

	size_t getPositionCount(const SquareMap<DynamicMagic>& map) {
		return std::ranges::fold_left(map.get(), 0, [](auto acc, const auto& m) {
			return acc + m.possiblePositions.size();
		});
	}

	void generateMagicBitboardTable() {
		MagicMaps<DynamicMagic> magicMaps;

		for (auto square : SQUARE_ARRAY) {
			mapSquare(magicMaps.orthogonalMoveMap, square, orthogonalMoveGenerator);
			mapSquare(magicMaps.diagonalMoveMap, square, diagonalMoveGenerator);
		}

		nlohmann::json j;
		j["orthogonal_move_map"] = magicMaps.orthogonalMoveMap.get();
		j["diagonal_move_map"] = magicMaps.diagonalMoveMap.get();
		j["position_count"] = getPositionCount(magicMaps.orthogonalMoveMap) + getPositionCount(magicMaps.diagonalMoveMap);
		auto tablePath = getTablePath();
		std::ofstream file{ tablePath };
		file << j.dump(2);
	}

	MagicMaps<StaticMagic> loadMaps() {
		std::ifstream file{ getTablePath() };
		auto json = nlohmann::json::parse(file);

		MagicMaps<StaticMagic> ret;

		auto positionCount = json["position_count"].get<size_t>();
		AllPositions::init(positionCount);

		ret.orthogonalMoveMap = json["orthogonal_move_map"].get<SquareMap<StaticMagic>::Buffer>();
		ret.diagonalMoveMap   = json["diagonal_move_map"].get<SquareMap<StaticMagic>::Buffer>();
		return ret;
	}
	const MagicMaps<StaticMagic>& getMaps() {
		static const auto maps = loadMaps();
		return maps;
	}

	MoveGen getSlidingMovesImpl(const SquareMap<StaticMagic>& map, Bitboard movingPieces, Bitboard empty) {
		MoveGen ret;

		auto currSquare = None;
		while (nextSquare(movingPieces, currSquare)) {
			auto blockers = ~empty & ~currSquare;
			auto allSquares = map[currSquare][blockers];
			ret |= MoveGen{ allSquares & empty, allSquares & ~empty };
		}
		return ret;
	}
	MoveGen BishopAttackGenerator::operator()(Bitboard movingPieces, Bitboard empty) const {
		return getSlidingMovesImpl(getMaps().diagonalMoveMap, movingPieces, empty);
	}
	MoveGen RookAttackGenerator::operator()(Bitboard movingPieces, Bitboard empty) const {
		return getSlidingMovesImpl(getMaps().orthogonalMoveMap, movingPieces, empty);
	}
}