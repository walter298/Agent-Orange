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
	SquareMap<PieceDestinationSquareData> calcAllySquareMap(const PieceState& allies, const PieceState& enemies, Bitboard enemySquares) {
		SquareMap<PieceDestinationSquareData> ret;

		PieceLocationData pieceLocations{ allies[King], allies.calcAllLocations(), enemies.calcAllLocations() };
		auto kingAttackerData = calcAttackers(IsWhite, enemies, pieceLocations.empty, pieceLocations.allyKing);

		auto attackedAllies = enemySquares & allies.calcAllLocations();
		auto pinMap = calcPinnedAllies(allies, enemies, attackedAllies, kingAttackerData, pieceLocations);

		forEachDestSquare<IsWhite>(allies, pinMap, kingAttackerData, pieceLocations, [&](Square square, Piece piece, const MoveGen& destSquares) {
			ret[square] = { piece, destSquares };
		});

		return ret;
	}

	DestinationSquares calcDestinationSquareMap(const Position& pos, const PositionData& posData) {
		DestinationSquares ret;

		auto [white, black] = pos.getColorSides();
		ret.whiteDestSquareMap = calcAllySquareMap<true>(white, black, posData.blackSquares.allDestSquares);
		ret.blackDestSquareMap = calcAllySquareMap<false>(black, white, posData.whiteSquares.allDestSquares);

		return ret;
	}
}