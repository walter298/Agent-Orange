export module Chess.MoveSearch:MoveHasher;

export import Chess.Move;

namespace chess {
	export struct MoveHasher {
		size_t operator()(const Move& move) const;
	};
}