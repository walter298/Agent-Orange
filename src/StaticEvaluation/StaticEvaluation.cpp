module Chess.Evaluation;

import std;

//redundant import, compiler bug mandates it
import Chess.PieceMap;
import Chess.Position;
import Chess.Profiler;
import Chess.RankCalculator;
import Chess.Rating;

namespace chess {
	const PieceMap<Rating> pieceRatings{
		{
			{ Queen, 90.0 },
			{ Rook, 50.0 },
			{ Bishop, 33.3 },
			{ Knight, 30.0 },
			{ Pawn, 10.0 }
		}
	};

	Rating getPieceRating(const PieceState& pieces) {
		Rating ret = 0.0;

		for (const auto& piece : ALL_PIECE_TYPES | std::views::drop(1)) { //don't count the king
			ret += (pieceRatings[piece] * static_cast<Rating>(std::popcount(pieces[piece])));;
		}

		return ret;
	}

	Rating calcMaterialRating(const Position& pos) {
		auto [white, black] = pos.getColorSides();
		return getPieceRating(white) - getPieceRating(black);
	}

	/*bool isPassedPawn(Square pawn, Bitboard enemyPawns) {
		auto rank  = rankOf(pawn);
		auto left  = std::clamp(rank - 1, 0, 7);
		auto right = std::clamp(rank + 1, 0, 7);

		Bitboard squaresAbove = 0;
		
	}*/

	template<bool MovingDown = false>
	Rating calcPawnAdvancementRatingImpl(Bitboard pawns) {
		Rating ret = 0.0;

		auto currSquare = Square::None;
		while (nextSquare(pawns, currSquare)) {
			auto rank = static_cast<Rating>(rankOf(currSquare)) + 1.0;
			if constexpr (MovingDown) {
				rank = 9.0 - rank;
			}
			auto pawnValue = rank * 0.001;
			ret += pawnValue;
		}

		return ret;
	}

	Rating calcPawnAdvancementRating(const Position& pos) {
		auto [white, black] = pos.getColorSides();
		return calcPawnAdvancementRatingImpl(white[Pawn]) - calcPawnAdvancementRatingImpl<true>(black[Pawn]);
	}

	Rating staticEvaluation(const Position& pos) {
		static MaybeProfiler profiler{ "staticEvaluation" };
		ProfilerLock l{ profiler };
		return calcMaterialRating(pos) + calcPawnAdvancementRating(pos);
	}

	Rating getPieceRating(Piece piece) {
		return pieceRatings[piece];
	}
}
