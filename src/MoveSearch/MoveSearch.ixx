export module Chess.MoveSearch;

export import std;

export import :MoveSearchTests;
import Chess.Position;
import Chess.SafeInt;

namespace chess {
	export std::optional<Move> findBestMove(const Position& pos, SafeUnsigned<std::uint8_t> depth);
}