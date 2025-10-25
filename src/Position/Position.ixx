export module Chess.Position;

import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.Square;

import Chess.PieceMap;
import Chess.RankCalculator;

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
			std::ranges::transform(m_data, m_data.begin(), [](auto& bitboard) {
				return 0;
			});
			m_canCastleKingside = true;
			m_canCastleQueenside = true;
			doubleJumpedPawn = Square::None;
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
			Bitboard allyPawnRank     = 0;
			Bitboard jumpedAllyPawnRank = 0;
		};
	public:
		using MutableTurnData   = TurnData<PieceState>;
		using ImmutableTurnData = TurnData<const PieceState>;
	private:
		PieceState m_whitePieces;
		PieceState m_blackPieces;
		
		bool m_isWhiteMoving = true;

		template<typename MaybeConstPieceState>
		TurnData<MaybeConstPieceState> getTurnDataImpl(this auto&& self) {
			if (self.m_isWhiteMoving) {
				return TurnData<MaybeConstPieceState>{
					self.m_whitePieces, self.m_blackPieces,
					WHITE_KINGSIDE, WHITE_QUEENSIDE,
					BLACK_KINGSIDE, BLACK_QUEENSIDE,
					self.m_isWhiteMoving,
					calcRank<2>(), calcRank<4>()
				};
			} else {
				return TurnData<MaybeConstPieceState>{
					self.m_blackPieces, self.m_whitePieces,
					BLACK_KINGSIDE, BLACK_QUEENSIDE,
					WHITE_KINGSIDE, WHITE_QUEENSIDE,
					self.m_isWhiteMoving,
					calcRank<7>(), calcRank<5>()
				};
			}
		}

		void parseBoard(std::string_view board);
		void parseCastlingPrivileges(std::string_view castlingPrivileges);
		void parseEnPessantSquare(std::string_view enPessantPriviliges);
	public:
		static constexpr std::string_view STARTING_FEN_STRING = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

		Position() = default;
		Position(Position&&) noexcept = default;
		Position(const Position&) = default;
		Position& operator=(const Position&) = default;

		Position(const Position& pos, const Move& move) {
			*this = pos;
			this->move(move);
		}

		void setStartPos();
		void setFen(std::string_view fen);

		void move(const Move& move);
		void move(std::string_view moveStr);

		TurnData<const PieceState> getTurnData() const {
			return getTurnDataImpl<const PieceState>();
		}
		TurnData<PieceState> getTurnData()  {
			return getTurnDataImpl<PieceState>();
		}
		auto getColorSides(this auto&& self) {
			return std::tie(self.m_whitePieces, self.m_blackPieces);
		}
	};
}