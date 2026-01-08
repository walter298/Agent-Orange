export module Chess.MoveSearch;

export import std;

import Chess.Position;
import Chess.SafeInt;
import Chess.Position.RepetitionMap;

export import :MoveSearchTests;

namespace chess {
	struct AsyncSearchState;

	export class AsyncSearch {
	private:
		std::shared_ptr<AsyncSearchState> m_state;
	public:
		AsyncSearch();

		std::optional<Move> findBestMove(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap);
		void cancel();
	};
}