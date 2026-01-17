export module Chess.MoveGeneration:Pin;

import Chess.PieceType;
import Chess.Position.PieceState;
import Chess.Square;

import :PieceAttackers;
import :PieceLocations;

export namespace chess {
	struct PinData {
		bool mustMoveInPinRay = false;
		Bitboard pinRay = 0; //does not include the pinner square
		Square pinnerSquare = Square::None;
		Piece pinnerPieceType = Piece::None;
	};
	using PinMap = SquareMap<PinData>;

	PinMap calcPinnedAllies(const PieceState& allies, const PieceState& enemies, const Bitboard attackedAllies,
		const AttackerData& kingAttackerData, const PieceLocationData& pieceLocations);
}