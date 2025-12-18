export module Chess.Bitboard;

import std;

export namespace chess {
	using Bitboard = std::uint64_t;
	constexpr Bitboard ALL_SQUARES = ~Bitboard{ 0 };

	constexpr Bitboard operator""_bb(Bitboard board) {
		return board;
	}
}