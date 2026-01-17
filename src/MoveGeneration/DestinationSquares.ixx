export module Chess.MoveGeneration:DestinationSquares;

export import :PieceLocations;

export import Chess.Position.PieceState;
export import Chess.Position;
export import Chess.Square;

import :PawnMoveGeneration;
import :KnightMoveGeneration;
import :SlidingMoveGenerators;
import :Pin;

namespace chess {
	template<typename MoveGenerator>
	MoveGen calcDestSquares(Square piecePos, const PinMap& pinMap, const AttackerData& kingAttackerData, 
		const PieceLocationData& pieceLocations, MoveGenerator moveGen)
	{
		auto destSquares = [&] {
			if constexpr (PawnMoveGenerator<MoveGenerator>) {
				return moveGen(makeBitboard(piecePos), pieceLocations.empty, pieceLocations.enemies);
			} else {
				return moveGen(makeBitboard(piecePos), pieceLocations.empty);
			}
		}();

		if (pinMap[piecePos].mustMoveInPinRay) {
			destSquares.emptyDestSquares &= pinMap[piecePos].pinRay;
			destSquares.nonEmptyDestSquares &= makeBitboard(pinMap[piecePos].pinnerSquare);
		}

		auto attackers = kingAttackerData.attackers.calcAllLocations();
		if (attackers) {
			destSquares &= (kingAttackerData.rays | attackers);
		}

		return destSquares;
	}

	//template<typename T>
	//concept DestinationSquareInvocable = std::invocable<Square, Piece, const MoveGen&>;

	template<typename MoveGenerator, typename Action>
	void forEachDestSquareImpl(const PinMap& allyPinMap, const AttackerData& kingAttackerData,
		Bitboard pieces, Piece pieceType, const PieceLocationData& pieceLocations, MoveGenerator moveGenerator, Action action)
	{
		auto currSquare = Square::None;
		while (nextSquare(pieces, currSquare)) {
			auto destSquares = calcDestSquares(currSquare, allyPinMap, kingAttackerData, pieceLocations, moveGenerator);
			action(currSquare, pieceType, destSquares);
		}
	}

	template<bool IsWhite, typename Action>
	void forEachDestSquare(const PieceState& allies, const PinMap& pinMap, const AttackerData& kingAttackerData, 
		const PieceLocationData& pieceLocations, Action action)
	{
		forEachDestSquareImpl(pinMap, kingAttackerData, allies[Queen], Queen, pieceLocations, queenMoveGenerator, action);
		forEachDestSquareImpl(pinMap, kingAttackerData, allies[Rook], Rook, pieceLocations, rookMoveGenerator, action);
		forEachDestSquareImpl(pinMap, kingAttackerData, allies[Bishop], Bishop, pieceLocations, bishopMoveGenerator, action);
		forEachDestSquareImpl(pinMap, kingAttackerData, allies[Knight], Knight, pieceLocations, knightMoveGenerator, action);
		if constexpr (IsWhite) {
			forEachDestSquareImpl(pinMap, kingAttackerData, allies[Pawn], Pawn, pieceLocations, whitePawnMoveGenerator, action);
		} else {
			forEachDestSquareImpl(pinMap, kingAttackerData, allies[Pawn], Pawn, pieceLocations, blackPawnMoveGenerator, action);
		}
	}

	export struct PieceDestinationSquareData {
		Piece piece = Piece::None;
		MoveGen destSquares;
	};

	export struct DestinationSquares {
		SquareMap<PieceDestinationSquareData> whiteDestSquareMap;
		SquareMap<PieceDestinationSquareData> blackDestSquareMap;
	};
	export DestinationSquares calcDestinationSquareMap(const Position& pos, const PositionData& posData);
}