export module Chess.AttackMap;

export import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.Position;
export import Chess.Rating;

export namespace chess {
    class AttackMap {
    private:
        Bitboard m_attackedSquares = 0;
        Rating m_threatenedMaterial = 0_rt;
    public:
    	AttackMap(Bitboard attackedSquares, const PieceState& allies);
        Rating materialChange(const Move& move) const;
    };
}