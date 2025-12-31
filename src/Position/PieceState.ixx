export module Chess.Position.PieceState;

import std;

export import Chess.Bitboard;
export import Chess.PieceMap;
export import Chess.Position.Castling;
export import Chess.Square;

namespace chess {
	export struct PieceState : public PieceMap<Bitboard> {
	private:
	public:
		CastlingPrivileges castling;
		Square doubleJumpedPawn = Square::None;

		constexpr Bitboard calcAllLocations() const {
			return m_data[0] | m_data[1] | m_data[2] | m_data[3] | m_data[4] | m_data[5];
		}
		Piece findPiece(Square square) const;

		void clear() {
			std::ranges::transform(m_data, m_data.begin(), [](auto&) {
				return 0;
			});
			castling.reset();
			doubleJumpedPawn = Square::None;
		}
	};
}