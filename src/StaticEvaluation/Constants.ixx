export module Chess.Evaluation:Constants;

import Chess.Rating;
import Chess.PieceMap;

namespace chess {
	constexpr auto QUEEN_RATING = 9_rt;
	constexpr auto ROOK_RATING = 5_rt;
	constexpr auto BISHOP_RATING = 3.3_rt;
	constexpr auto KNIGHT_RATING = 3_rt;
	constexpr auto PAWN_RATING = 1_rt;
	constexpr auto PAWN_ADVANCEMENT_RATING = 0.004_rt;
	constexpr auto ATTACKED_PIECE_RATING = 0.001_rt;
	constexpr auto PAWN_ISLAND_PENALTY = -0.02_rt;
	constexpr auto PIECE_PROXIMITY_FACTOR = -0.0006_rt;
	constexpr auto DESTINATION_SQUARE_PROXIMITY_FACTOR = -0.0002_rt;
	constexpr auto CASTLE_RATING = 0.2_rt;
	constexpr auto OPTIMAL_KNIGHT_SQUARES = 4;
	constexpr auto OPTIMAL_BISHOP_SQUARES = 6;
	constexpr auto OPTIMAL_QUEEN_SQUARES = 10;
	constexpr auto OPTIMAL_ROOK_SQUARES = 6;

	const PieceMap<Rating> pieceRatings{
		{
			{ Queen, QUEEN_RATING },
			{ Rook, ROOK_RATING },
			{ Bishop, BISHOP_RATING },
			{ Knight, KNIGHT_RATING },
			{ Pawn, PAWN_RATING }
		}
	};
}