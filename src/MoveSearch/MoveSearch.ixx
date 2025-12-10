export module Chess.MoveSearch;

export import std;

export import :MoveSearchTests;
import Chess.Position;

namespace chess {
	export std::optional<Move> findBestMove(const Position& pos, std::uint8_t depth);
}