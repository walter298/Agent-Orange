export module Chess.Position;

import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.Square;
export import Chess.Position.PieceState;
export import Chess.PositionCommand;

import Chess.PieceMap;
import Chess.RankCalculator;

namespace chess {

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
	public:
		Position() = default;
		Position(Position&&) noexcept = default;
		Position(const Position&) = default;
		Position& operator=(const Position&) = default;

		Position(const Position& pos, const Move& move) {
			*this = pos;
			this->move(move);
		}

		void setPos(const PositionCommand& positionCommand);

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
		bool isWhite() const {
			return m_isWhiteMoving;
		}
	};
}