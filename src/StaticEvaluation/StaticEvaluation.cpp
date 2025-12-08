module Chess.Evaluation;

import std;

//redundant import, compiler bug mandates it
import Chess.PieceMap;
import Chess.Position;
import Chess.Profiler;
import Chess.RankCalculator;
import Chess.Rating;

namespace chess {
	constexpr auto QUEEN_RATING = 90_rt;
	constexpr auto ROOK_RATING = 50_rt;
	constexpr auto BISHOP_RATING = 33_rt;
	constexpr auto KNIGHT_RATING = 30_rt;
	constexpr auto PAWN_RATING = 10_rt;
	constexpr auto PAWN_ADVANCEMENT_RATING = 0.001_rt;
	constexpr auto ATTACKED_PIECE_RATING = 0.01_rt;

	const PieceMap<Rating> pieceRatings{
		{
			{ Queen, QUEEN_RATING },
			{ Rook, ROOK_RATING },
			{ Bishop, BISHOP_RATING },
			{ Knight, KNIGHT_RATING },
			{ Pawn, PAWN_RATING }
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

	template<bool MovingDown = false>
	Rating calcPawnAdvancementRatingImpl(Bitboard pawns) {
		Rating ret = 0.0;

		auto currSquare = Square::None;
		while (nextSquare(pawns, currSquare)) {
			auto rank = rankOf(currSquare) + 1;
			if constexpr (MovingDown) {
				rank = 9 - rank;
			}
			auto pawnValue = static_cast<Rating>(rank) * PAWN_ADVANCEMENT_RATING;
			ret += pawnValue;
		}

		return ret;
	}

	Rating calcPawnAdvancementRating(const Position& pos) {
		auto [white, black] = pos.getColorSides();
		return calcPawnAdvancementRatingImpl(white[Pawn]) - calcPawnAdvancementRatingImpl<true>(black[Pawn]);
	}

	Rating calcAttackRating(const Position& pos, const PositionData& posData) {
		auto [white, black] = pos.getColorSides();

		auto getAttackedPiecesRating = [&](const PieceState& pieceState) -> Rating {
			auto attackedPieces = pieceState.calcAllLocations() & posData.attackedPieces;
			auto attackedPieceCount = std::popcount(attackedPieces);
			return static_cast<Rating>(attackedPieceCount) * ATTACKED_PIECE_RATING;
		};
		return getAttackedPiecesRating(white) - getAttackedPiecesRating(black);
	}

	Rating staticEvaluation(const Position& pos, const PositionData& posData) {
		ProfilerLock l{ getStaticEvaluationProfiler() };
		return calcMaterialRating(pos) + calcPawnAdvancementRating(pos) + calcAttackRating(pos, posData);
	}

	Rating getPieceRating(Piece piece) {
		return pieceRatings[piece];
	}
}
