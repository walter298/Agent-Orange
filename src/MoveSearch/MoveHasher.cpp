module;

#include <boost/functional/hash.hpp>

module Chess.MoveSearch:MoveHasher;

namespace chess {
	size_t MoveHasher::operator()(const Move& move) const {
		size_t hash = 0;
		boost::hash_combine(hash, move.from);
		boost::hash_combine(hash, move.to);
		boost::hash_combine(hash, move.movedPiece);
		boost::hash_combine(hash, move.capturedPiece);
		boost::hash_combine(hash, move.promotionPiece);
		boost::hash_combine(hash, move.capturedPawnSquareEnPassant);
		return hash;
	}
}