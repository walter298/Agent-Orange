export module Chess.PieceType;

import std;

namespace chess {
	export enum Piece : std::uint8_t {
		King,
		Queen,
		Rook,
		Bishop,
		Knight,
		Pawn,
		None
	};

	export constexpr std::array ALL_PIECE_TYPES = { King, Queen, Rook, Bishop, Knight, Pawn };
}