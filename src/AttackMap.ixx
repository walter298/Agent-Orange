export module Chess.AttackMap;

export import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.Rating;

export namespace chess {
    class AttackMap {
    private:
        Bitboard m_attackedSquares = 0;
    public:
        constexpr AttackMap(Bitboard attackedSquares) : m_attackedSquares{ attackedSquares } {}
        Rating materialChange(const Move& move) const;
    };
}