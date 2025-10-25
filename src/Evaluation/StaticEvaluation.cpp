module Chess.Evaluation:StaticEvaluation;

import std;

//redundant import, compiler bug mandates it
import Chess.PieceMap;
import Chess.Position; 
import :Rating;

namespace chess {
	Rating getPieceRating(const PieceState& pieces) {
		const PieceMap<Rating> pieceRatings{
			{
				{ Queen, 9.0 },
				{ Rook, 5.0 },
				{ Bishop, 3.3 },
				{ Knight, 3.0 },
				{ Pawn, 1.0 }
			}
		};

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
