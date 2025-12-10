export module Chess.MoveSearch:MovePriority;

import std;
export import Chess.Move;

export namespace chess {
	class MovePriority {
	private:
		Move m_move;
		std::uint8_t m_recommendedDepth = 0;
		bool m_inAttackSequence = false;
	public:
		MovePriority(const Move& move, std::uint8_t recommendedDepth) :
			m_move{ move }, m_recommendedDepth{ recommendedDepth },
			m_inAttackSequence{ move.capturedPiece != Piece::None }
		{
		}

		const Move& getMove() const {
			return m_move;
		}
		std::uint8_t getDepth() const {
			return m_recommendedDepth;
		}
		bool inAttackSequence() const {
			return m_inAttackSequence;
		}

		std::string getString() const {
			return std::format("[{}, {}]", m_move.getUCIString(), static_cast<int>(m_recommendedDepth));
		}
	};
}