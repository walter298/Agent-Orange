export module Chess.Position;

import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.Square;

import Chess.PieceMap;

namespace chess {
	export class SpecialMoveData {
	private:
		std::uint8_t m_movementData = 0;
	public:
		bool canCastleKingside() const {
			return 1 & m_movementData;
		}
		bool canCastleQueenside() const {
			return 0b10 & m_movementData;
		}
		Square getEnPessantSquare() const {
			return static_cast<Square>(m_movementData >> 2);
		}
	};

	export struct PieceState : public PieceMap<Bitboard> {
		constexpr Bitboard calcAllLocations() const {
			return m_data[0] | m_data[1] | m_data[2] | m_data[3] | m_data[4] | m_data[5];
		}
		Piece findPiece(Square square) const;
	};

	export class Position {
	public:
		struct Sides {
			const PieceState& allies;
			const PieceState& enemies;
			bool isWhite = true;
		};
	private:
		PieceState m_whitePieces;
		PieceState m_blackPieces;
		SpecialMoveData m_whiteSpecialMoveData;
		SpecialMoveData m_blackSpecialMoveData;
		bool m_isWhiteMoving = true;

		auto getSidesMutable() {
			return m_isWhiteMoving ? std::tie(m_whitePieces, m_blackPieces) : std::tie(m_blackPieces, m_whitePieces);
		}

		Position() = default;
		static Position startPos();
	public:
		static Position fromStartPos(std::string_view fen);
		static Position fromFENString(std::string_view fen);

		void move(const Move& move);
		void move(std::string_view moveStr);

		Sides getTurnSides() const {
			return m_isWhiteMoving ? Sides{ m_whitePieces, m_blackPieces, true } : Sides{ m_blackPieces, m_whitePieces, false };
		}
		auto getColorSides() const {
			return std::tie(m_whitePieces, m_blackPieces);
		}
	};
}