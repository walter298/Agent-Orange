module Chess.Evaluation:PieceDevelopment;

import :Constants;

import Chess.DebugPrint;
import Chess.MoveGeneration;

namespace chess {
	Rating calcPieceDevelopmentRatingImpl(const PieceState& pieces, const SquareMap<PieceDestinationSquareData>& destSquareMap) {
		auto ret = 0_rt;
		auto developedPieceCount = 0;

		for (auto square : SQUARE_ARRAY) {
			const auto& destSquareData = destSquareMap[square];
			if (destSquareData.piece == Piece::None) {
				continue;
			}
			auto squareCount = std::popcount(destSquareData.destSquares.all());
			auto mobilityScore = (squareCount - optimalDestinationSquareCounts[destSquareData.piece]) * MOBILITY_SQUARE_RATING;
			ret += mobilityScore;
			if (mobilityScore > 0_rt) {
				developedPieceCount++;
				ret *= developedPieceCount * MOBILITY_DISTRIBUTION_RATING;
			}
		}

		return ret;
	}

	Rating calcPieceDevelopmentRating(const Position& pos, const PositionData& posData) {
		auto [white, black] = pos.getColorSides();
		auto [whiteDestSquareMap, blackDestSquareMap] = calcDestinationSquareMap(pos, posData);

		return calcPieceDevelopmentRatingImpl(white, whiteDestSquareMap) -
			   calcPieceDevelopmentRatingImpl(black, blackDestSquareMap);
	}
}