export module Chess.Move;

import std;
export import Chess.Square;
export import Chess.PieceType;

namespace chess {
	export struct Move {
		Square from;
		Square to;
		Piece movedPiece;
		Piece capturedPiece;
		Piece promotedPiece;

		Move(Square from, Square to, Piece movedPiece, Piece capturedPiece, Piece promotedPiece)
			: from{ from }, to{ to }, movedPiece{ movedPiece }, capturedPiece{ capturedPiece },
			promotedPiece{ promotedPiece }
		{
		}

		Move(Square from, Square to, Piece movedPiece, Piece capturedPiece)
			: Move{ from, to, movedPiece, capturedPiece, Piece::None }
		{
		}
		
		std::string getUCIString() const;
	};
}