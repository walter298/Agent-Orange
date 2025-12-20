export module Chess.MoveSearch:MovePriority;

import std;

export import Chess.Bitboard;
export import Chess.Evaluation;
export import Chess.Move;
export import Chess.SafeInt;
export import Chess.Rating;

namespace chess {
	Rating calcExchangeRating(const Move& m, Bitboard enemySquares) {
		auto ret = 0_rt;

		if (makeBitboard(m.to) & enemySquares) {
			ret -= getPieceRating(m.movedPiece);
		} else if (m.promotionPiece != Piece::None) {
			ret += getPieceRating(m.promotionPiece);
		}
		if (m.capturedPiece != Piece::None) {
			ret += getPieceRating(m.capturedPiece);
		}
		return ret;
	}

	export class MovePriority {
	private:
		Move m_move;
		SafeUnsigned<std::uint8_t> m_recommendedDepth{ 0 };
		Rating m_exchangeRating = 0_rt;
	public:
		MovePriority(const Move& move, Bitboard enemySquares, SafeUnsigned<std::uint8_t> recommendedDepth) :
			m_move{ move }, m_recommendedDepth{ recommendedDepth },
			m_exchangeRating{ calcExchangeRating(move, enemySquares) }
		{
		}

		const Move& getMove() const {
			return m_move;
		}
		SafeUnsigned<std::uint8_t> getDepth() const {
			return m_recommendedDepth;
		}
		Rating getExchangeRating() const {
			return m_exchangeRating;
		}
		std::string getString() const {
			return std::format("[{}, {}]", m_move.getUCIString(), static_cast<unsigned int>(m_recommendedDepth.get()));
		}
	};
}