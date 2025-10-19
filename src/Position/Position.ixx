export module Chess.Position;

import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.Square;

import Chess.PieceMap;

namespace chess {
	export struct PieceState : public PieceMap<Bitboard> {
	private:
		std::uint8_t m_movementData = 0b11111111;
	public:
		constexpr Bitboard calcAllLocations() const {
			return m_data[0] | m_data[1] | m_data[2] | m_data[3] | m_data[4] | m_data[5];
		}
		Piece findPiece(Square square) const;

		constexpr bool canCastleKingside() const {
			return 1 & m_movementData;
		}
		constexpr void disallowKingsideCastling() {
			m_movementData &= ~std::uint8_t{ 1 };
		}
		constexpr bool canCastleQueenside() const {
			return 0b10 & m_movementData;
		}
		constexpr void disallowQueensideCastling() {
			m_movementData &= ~std::uint8_t{ 0b10 };
		}
		constexpr Square getEnPessantSquare() const {
			return static_cast<Square>(m_movementData >> 2);
		}
	};

	export class Position {
	private:
		struct CastleMove {
			Square kingTo;
			Square rookFrom;
			Square rookTo;
			Bitboard squaresBetweenRookAndKing;
		};
		static constexpr CastleMove WHITE_KINGSIDE  = { G1, H1, F1, makeBitboard(F1, G1) };
		static constexpr CastleMove WHITE_QUEENSIDE = { C1, A1, D1, makeBitboard(B1, C1, D1) };
		static constexpr CastleMove BLACK_KINGSIDE  = { G8, H8, F8, makeBitboard(F8, G8) };
		static constexpr CastleMove BLACK_QUEENSIDE = { C8, A8, D8, makeBitboard(B8, C8, D8) };

		template<typename MaybeConstPieceState>
		struct TurnData {
			MaybeConstPieceState& allies;
			MaybeConstPieceState& enemies;
			const CastleMove& allyKingside;
			const CastleMove& allyQueenside;
			const CastleMove& enemyKingside;
			const CastleMove& enemyQueenside;
			bool isWhite = true;
		};
	public:
		using MutableTurnData   = TurnData<PieceState>;
		using ImmutableTurnData = TurnData<const PieceState>;
	private:
		PieceState m_whitePieces;
		PieceState m_blackPieces;
		
		bool m_isWhiteMoving = true;

		template<typename MaybeConstPieceState>
		TurnData<MaybeConstPieceState> getSidesImpl(this auto&& self) {
			if (self.m_isWhiteMoving) {
				return TurnData<MaybeConstPieceState>{
					self.m_whitePieces, self.m_blackPieces,
					WHITE_KINGSIDE, WHITE_QUEENSIDE,
					BLACK_KINGSIDE, BLACK_QUEENSIDE,
					self.m_isWhiteMoving
				};
			} else {
				return TurnData<MaybeConstPieceState>{
					self.m_blackPieces, self.m_whitePieces,
					BLACK_KINGSIDE, BLACK_QUEENSIDE,
					WHITE_KINGSIDE, WHITE_QUEENSIDE,
					self.m_isWhiteMoving
				};
			}
		}

		static Position startPos();
	public:
		static constexpr std::string_view STARTING_FEN_STRING = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		Position() = default;


		void setStartPos();
		void setFen(std::string_view fen);

		static Position fromStartPos(std::string_view fen);
		static Position fromFENString(std::string_view fen);

		void move(const Move& move);
		void move(std::string_view moveStr);

		TurnData<const PieceState> getTurnData() const {
			return getSidesImpl<const PieceState>();
		}
		TurnData<PieceState> getTurnData()  {
			return getSidesImpl<PieceState>();
		}
		auto getColorSides() const {
			return std::tie(m_whitePieces, m_blackPieces);
		}
	};
}