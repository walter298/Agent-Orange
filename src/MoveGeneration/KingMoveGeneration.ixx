export module Chess.LegalMoveGeneration:KingMoveGeneration;

import Chess.Bitboard;
export import :MoveGen;

namespace chess {
	struct KingMoveGenerator {
		MoveGen operator()(Bitboard king, Bitboard empty) const;
	};
	export KingMoveGenerator kingMoveGenerator;
}