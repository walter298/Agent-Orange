export module Chess.RankCalculator;

export import Chess.Bitboard;

export namespace chess {
	template<int Rank>
	consteval Bitboard calcRank() requires(Rank >= 1 && Rank <= 8) {
		return Bitboard{ 0xFF } << ((Rank - 1) * 8);
	}

	constexpr Bitboard calcRank(int rankIndex) {
		return Bitboard{ 0xFF } << (rankIndex * 8);
	}

	template<int File>
	consteval Bitboard calcFile() {
		return 0x0101010101010101ull << (File - 1);
	}

	constexpr Bitboard calcFile(int fileIndex) {
		return 0x0101010101010101ull << fileIndex;
	}
}