export module Chess.Move;

import std;
export import Chess.Square;
export import Chess.PieceType;

namespace chess {
	export struct Move {
		Square from = Square::None;
		Square to = Square::None;
		Square capturedPawnSquareEnPassant = Square::None;
		Piece movedPiece = Piece::None;
		Piece capturedPiece = Piece::None;
		Piece promotionPiece = Piece::None;

		constexpr Move() = default;

		constexpr Move(Square from, Square to, Square enPessantSquare, Piece movedPiece, Piece capturedPiece, Piece promotedPiece)
			: from{ from }, to{ to }, capturedPawnSquareEnPassant{ enPessantSquare }, movedPiece { movedPiece }, capturedPiece{ capturedPiece },
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
				capturedPawnSquareEnPassant == other.capturedPawnSquareEnPassant;
		}

		bool isMaterialChange() const {
			return capturedPiece != Piece::None || promotionPiece != Piece::None;
		}
		
		std::string getUCIString() const;
	};
}