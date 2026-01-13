module;

#include <immintrin.h>

module Chess.MoveGeneration:TableStore;

import std;
import nlohmann.json;

import Chess.Direction;
import Chess.RankCalculator;

import :BMI;
import :BruteForceDirectionalMoveGeneration;
import :TableData;

namespace chess {
	void to_json(nlohmann::json& j, const DynamicBMI& m) {
		j[RAYS_KEY] = m.rays;
		j[POSITIONS_KEY] = m.possiblePositions;
	}

	static Bitboard ithSubBoard(Bitboard src, Bitboard i) {
		Bitboard subBoard = 0;
		auto s = Square::None;
		while (nextSquare(i, s)) {
			subBoard |= makeBitboard(getNthSetSquare(src, static_cast<int>(s)));
		}
		return subBoard;
	}

	template<std::invocable<Bitboard> Func>
	void forEachSubBoard(Bitboard board, Func f) {
		auto possiblePositionCount = 1 << std::popcount(board);
		for (Bitboard i = 0; i < possiblePositionCount; i++) {
			auto ithBoard = ithSubBoard(board, i);
			f(ithBoard);
		}
	}

	//note: calcFile<N> and calcRank<N> return the n-1th file and rank, respectively
	Bitboard trimEdges(Square movedPiece, Bitboard rays) {
		auto file = fileOf(movedPiece);
		if (file != 0) {
			rays &= ~calcFile<1>();
		}
		if (file != 7) {
			rays &= ~calcFile<8>();
		}

		auto rank = rankOf(movedPiece);
		if (rank != 0) {
			rays &= ~calcRank<1>();
		}
		if (rank != 7) {
			rays &= ~calcRank<8>();
		}
		return rays;
	}

	template<typename MoveGenerator>
	DynamicBMI makeMagicEntry(Square square, MoveGenerator moveGenerator) {
		auto pieceBoard = makeBitboard(square);
		auto empty = ALL_SQUARES & ~pieceBoard;
		auto rays = moveGenerator(pieceBoard, empty).all();

		DynamicBMI ret;
		ret.rays = trimEdges(square, rays);

		auto possiblePositionCount = 1 << std::popcount(ret.rays);
		ret.possiblePositions.resize(static_cast<size_t>(possiblePositionCount));

		forEachSubBoard(ret.rays, [&](Bitboard pieces) {
			auto index = _pext_u64(pieces, ret.rays);
			auto destSquares = moveGenerator(pieceBoard, ~pieces).all();
			ret.possiblePositions[index] = destSquares;
		});

		return ret;
	}

	size_t getPositionCount(const SquareMap<DynamicBMI>& map) {
		return std::ranges::fold_left(map.get(), 0, [](auto acc, const auto& m) {
			return acc + m.possiblePositions.size();
		});
	}

	void storeBMITable() {
		MagicMaps<DynamicBMI> magicMaps;

		//populate the maps
		for (auto square : SQUARE_ARRAY) {
			magicMaps.orthogonalMoveMap[square] = makeMagicEntry(square, orthogonalMoveGenerator);
			magicMaps.diagonalMoveMap[square]   = makeMagicEntry(square, diagonalMoveGenerator);
		}

		//serialize the json
		nlohmann::json j;
		j[ORTHOGONAL_MOVE_MAP_KEY] = magicMaps.orthogonalMoveMap.get();
		j[DIAGONAL_MOVE_MAP_KEY] = magicMaps.diagonalMoveMap.get();
		j[TOTAL_POSITION_COUNT_KEY] = getPositionCount(magicMaps.orthogonalMoveMap) + getPositionCount(magicMaps.diagonalMoveMap);

		//store the json in a file
		auto tablePath = getTablePath();
		std::ofstream file{ tablePath };
		file << j.dump(2);
	}
}