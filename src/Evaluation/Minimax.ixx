export module Chess.Evaluation:Minimax;

import Chess.Position;
import :Rating;

namespace chess {
	export Rating minimax(const Position& pos, int depth, bool maximizing);
}