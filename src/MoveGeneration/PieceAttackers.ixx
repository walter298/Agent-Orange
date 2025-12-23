export module Chess.MoveGeneration:PieceAttackers;

import std;

export import Chess.Position.PieceState;
export import Chess.Square;

export import :PieceLocations;


namespace chess {
	export struct AttackerData {
		PieceState attackers;
		Bitboard rays = 0;
		Bitboard indirectRays = 0;

		bool hasMultipleAttackers() const {
			return std::popcount(attackers.calcAllLocations()) > 1;
		}

		Bitboard allRays() const {
			return rays | indirectRays;
		}
	};

	AttackerData calcSlidingAttackers(const PieceState& enemies, Bitboard empty, Bitboard attackedPiece);
	export AttackerData calcAttackers(bool isWhite, const PieceState& enemies, Bitboard empty, Bitboard attackedPiece);
}