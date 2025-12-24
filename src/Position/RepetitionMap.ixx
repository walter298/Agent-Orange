export module Chess.Position.RepetitionMap;

import Chess.Position;

export namespace chess {
	namespace repetition {
		void push(const Position& pos);
		void pop();
		int getPositionCount(const Position& pos);
		void clear();
		int getTotalPositionCount();
	}
}