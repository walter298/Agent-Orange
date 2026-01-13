module Chess.Evaluation:PieceDevelopment;

import :Constants;

import Chess.DebugPrint;

namespace chess {
	Rating calcPieceDevelopmentRatingImpl(const PieceState& pieces, const PieceMap<MoveGen>& allDestSquares) {
		auto ret = 0_rt;

		auto nonKings = std::views::zip(ALL_PIECE_TYPES, pieces, allDestSquares) | std::views::drop(1);
		for (const auto& [pieceType, pieceLocations, destSquares] : nonKings) {
			auto pieceCount = std::popcount(pieceLocations);
			auto squareCount = std::popcount(destSquares.all());
			if (squareCount == 0) {
				ret += UNDEVELOPED_PIECE_PENALTY * pieceCount;
				continue;
			}
			auto optimalSquareCount = pieceCount * optimalDestinationSquareCounts[pieceType];
			auto undevelopedPieceCount = optimalSquareCount / squareCount;

			//debugPrint(std::format("Undeveloped {} count: {}"))
			ret += UNDEVELOPED_PIECE_PENALTY * undevelopedPieceCount;
		}

		return ret;
	}

	Rating calcPieceDevelopmentRating(const Position& pos, const PositionData& posData) {
//		auto [white, black] = pos.getColorSides();
//		return calcPieceDevelopmentRatingImpl(white, posData.whiteSquares) -
//			   calcPieceDevelopmentRatingImpl(black, posData.blackSquares);
		return 0;
	}
}