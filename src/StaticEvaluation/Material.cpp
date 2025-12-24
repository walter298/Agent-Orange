module Chess.Evaluation:Material;

import Chess.Position.PieceState;

import :Constants;

namespace chess {
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
}