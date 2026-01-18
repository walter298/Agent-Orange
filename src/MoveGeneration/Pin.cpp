module Chess.MoveGeneration:Pin;

namespace chess {
	PinData calcPinData(Square allySquare, const PieceState& enemies, const AttackerData& kingAttackerData,
		const PieceLocationData& pieceLocations)
	{
		PinData ret;

		auto newEmpty = pieceLocations.empty | makeBitboard(allySquare);
		auto newSlidingAttackers = calcSlidingAttackers(enemies, newEmpty, pieceLocations.allyKing);
		auto newAttacker = newSlidingAttackers.attackers.calcAllLocations() & ~kingAttackerData.attackers.calcAllLocations();

		if (!newAttacker) { //no new king attackers, piece is not pinned
			return ret;
		}
		auto newRay = newSlidingAttackers.allRays() & ~kingAttackerData.allRays();

		ret.mustMoveInPinRay = true;
		ret.pinRay = newRay;
		ret.pinnerSquare = nextSquare(newAttacker);
		ret.pinnerPieceType = enemies.findPiece(ret.pinnerSquare);

		return ret;
	}

	PinMap calcPinnedAllies(const PieceState& allies, const PieceState& enemies, const Bitboard attackedAllies,
		const AttackerData& kingAttackerData, const PieceLocationData& pieceLocations)
	{
		PinMap ret;

		auto calcPinnedAlliesImpl = [&](Bitboard pieces) {
			auto attackedPieces = pieces & attackedAllies;
			auto currSquare = Square::None;
			while (nextSquare(attackedPieces, currSquare)) {
				ret[currSquare] = calcPinData(currSquare, enemies, kingAttackerData, pieceLocations);
			}
		};
		calcPinnedAlliesImpl(allies[Queen]);
		calcPinnedAlliesImpl(allies[Rook]);
		calcPinnedAlliesImpl(allies[Bishop]);
		calcPinnedAlliesImpl(allies[Knight]);
		calcPinnedAlliesImpl(allies[Pawn]);

		return ret;
	}
}