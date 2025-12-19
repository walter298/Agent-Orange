module Chess.LegalMoveGeneration:TableLoader;

import std;
import nlohmann.json;

import :BMI;
import :TableData;

namespace chess {
	class AllPositions {
	private:
		static inline std::vector<Bitboard> m_allPositions;
		static inline size_t m_index = 0;
	public:
		static void init(size_t n) {
			m_allPositions.resize(n);
		}
		static const Bitboard* addPositions(const std::vector<Bitboard>& positions) {
			auto ret = m_allPositions.data() + m_index;
			std::ranges::copy(positions, m_allPositions.begin() + m_index);
			m_index += positions.size();
			return ret;
		}
	};

	void from_json(const nlohmann::json& j, StaticBMI& m) {
		auto possiblePositions = j[POSITIONS_KEY].get<std::vector<Bitboard>>();

		m.possiblePositions = AllPositions::addPositions(possiblePositions);
		m.rays = j["rays"].get<Bitboard>();
	}

	MagicMaps<StaticBMI> loadMaps() {
		std::ifstream file{ getTablePath() };
		auto json = nlohmann::json::parse(file);

		MagicMaps<StaticBMI> ret;

		auto positionCount = json[TOTAL_POSITION_COUNT_KEY].get<size_t>();
		AllPositions::init(positionCount);

		ret.orthogonalMoveMap = json[ORTHOGONAL_MOVE_MAP_KEY].get<SquareMap<StaticBMI>::Buffer>();
		ret.diagonalMoveMap   = json[DIAGONAL_MOVE_MAP_KEY].get<SquareMap<StaticBMI>::Buffer>();
		return ret;
	}
}