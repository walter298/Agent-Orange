export module Chess.Move;

import std;
export import Chess.Square;
export import Chess.PieceType;

namespace chess {
	export struct Move {
		Square from;
		Square to;
		Square enPessantSquare;
		Piece movedPiece;
		Piece capturedPiece;
		Piece promotionPiece;

		constexpr Move(Square from, Square to, Square enPessantSquare, Piece movedPiece, Piece capturedPiece, Piece promotedPiece)
			: from{ from }, to{ to }, enPessantSquare{ enPessantSquare }, movedPiece { movedPiece }, capturedPiece{ capturedPiece },
			promotionPiece{ promotedPiece }
		{
		}

		constexpr Move(Square from, Square to, Piece movedPiece, Piece capturedPiece, Piece promotedPiece)
			: Move{ from, to, Square::None, movedPiece, capturedPiece, promotedPiece }
		{
		}

		constexpr Move(Square from, Square to, Piece movedPiece, Piece capturedPiece)
			: Move{ from, to, movedPiece, capturedPiece, Piece::None }
		{
		}

		static consteval Move null() {
			return { Square::None, Square::None, Piece::None, Piece::None, Piece::None };
		}

		constexpr bool operator==(const Move& other) const {
			return from == other.from && to == other.to && movedPiece == other.movedPiece &&
				capturedPiece == other.capturedPiece && promotionPiece == other.promotionPiece &&
				enPessantSquare == other.enPessantSquare;
		}
		
		std::string getUCIString() const;
	};
}