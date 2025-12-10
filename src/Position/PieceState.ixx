export module Chess.Position.PieceState;

import std;

export import Chess.Bitboard;
export import Chess.PieceMap;
export import Chess.Square;

namespace chess {
	export struct PieceState : public PieceMap<Bitboard> {
	private:
		bool m_canCastleKingside = true;
		bool m_canCastleQueenside = true;
	public:
		Square doubleJumpedPawn = Square::None;

		constexpr Bitboard calcAllLocations() const {
			return m_data[0] | m_data[1] | m_data[2] | m_data[3] | m_data[4] | m_data[5];
		}
		Piece findPiece(Square square) const;

		constexpr bool canCastleKingside() const {
			return m_canCastleKingside;
		}
		constexpr void disallowKingsideCastling() {
			m_canCastleKingside = false;
		}
		constexpr bool canCastleQueenside() const {
			return m_canCastleQueenside;
		}
		constexpr void disallowQueensideCastling() {
			m_canCastleQueenside = false;
		}
		void clear() {
			std::ranges::transform(m_data, m_data.begin(), [](auto&) {
				return 0;
			});
			m_canCastleKingside = true;
			m_canCastleQueenside = true;
			doubleJumpedPawn = Square::None;
		}
	};
}