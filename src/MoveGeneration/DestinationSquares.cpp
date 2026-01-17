module Chess.MoveGeneration:DestinationSquares;

import :Pin;
import :PawnMoveGeneration;
import :SlidingMoveGenerators;
import :KnightMoveGeneration;

namespace chess {
	template<typename MoveGenerator>
	void calcDestSquaresImpl(SquareMap<MoveGen>& squares, const PinMap& allyPinMap, const AttackerData& kingAttackerData, 
		Bitboard pieces, const PieceLocationData& pieceLocations, MoveGenerator moveGenerator)
	{
		auto currSquare = Square::None;
		while (nextSquare(pieces, currSquare)) {
			squares[currSquare] = calcDestSquares(currSquare, allyPinMap, kingAttackerData, pieceLocations, moveGenerator);
		}
	}

	template<bool IsWhite>
	SquareMap<MoveGen> calcAllySquareMap(const PieceState& allies, const PieceState& enemies, Bitboard enemySquares) {
		SquareMap<MoveGen> ret;

		PieceLocationData pieceLocations{ allies[King], allies.calcAllLocations(), enemies.calcAllLocations() };
		auto kingAttackerData = calcAttackers(IsWhite, enemies, pieceLocations.empty, pieceLocations.allyKing);

		auto attackedAllies = enemySquares & allies.calcAllLocations();
		auto pinMap = calcPinnedAllies(allies, enemies, attackedAllies, kingAttackerData, pieceLocations);

		calcDestSquaresImpl(ret, pinMap, kingAttackerData, allies[Queen], pieceLocations, queenMoveGenerator);
		calcDestSquaresImpl(ret, pinMap, kingAttackerData, allies[Rook], pieceLocations, rookMoveGenerator);
		calcDestSquaresImpl(ret, pinMap, kingAttackerData, allies[Bishop], pieceLocations, bishopMoveGenerator);
		calcDestSquaresImpl(ret, pinMap, kingAttackerData, allies[Knight], pieceLocations, knightMoveGenerator);
		if constexpr (IsWhite) {
			calcDestSquaresImpl(ret, pinMap, kingAttackerData, allies[Pawn], pieceLocations, whitePawnMoveGenerator);
		} else {
			calcDestSquaresImpl(ret, pinMap, kingAttackerData, allies[Pawn], pieceLocations, blackPawnAttackGenerator);
		}

		return ret;
	}

	DestinationSquares getDestinationSquares(const Position& pos, const PositionData& posData) {
		DestinationSquares ret;

		//auto [white, black] = pos.getColorSides();
		//ret.whiteDestSquares = calcAllySquareMap<true>(white, black, posData.blackSquares.allDestSquares);
		//ret.blackSquares     = calcAllySquareMap<false>(black, white, posData.whiteSquares.allDestSquares);

		return ret;
	}
}