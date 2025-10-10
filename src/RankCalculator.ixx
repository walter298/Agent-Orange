export module Chess.RankCalculator;

export import Chess.Bitboard;

export namespace chess {
	template<int Rank>
	consteval Bitboard calcRank() requires(Rank >= 1 && Rank <= 8) {
		return Bitboard{ 0xFF } << ((Rank - 1) * 8);
	}
}