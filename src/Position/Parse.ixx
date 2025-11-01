export module Chess.Position:Parse;

import std;
import Chess.Position.PieceState;

export namespace chess {
	Piece parsePiece(char chr);
	void parseBoard(std::string_view board, PieceState& whitePieces, PieceState& blackPieces);
	void parseCastlingPrivileges(std::string_view castlingPrivileges, PieceState& white, PieceState& black);
	void parseEnPessantSquare(std::string_view enPessantSquareStr, bool isWhite, PieceState& enemies);
}