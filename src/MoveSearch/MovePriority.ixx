export module Chess.MoveSearch:MovePriority;

export import Chess.Move;

export namespace chess {
	class MovePriority {
	private:
		Move m_move;
		int m_recommendedDepth = 0;
		bool m_inAttackSequence = false;
	public:
		MovePriority(const Move& move, int recommendedDepth) :
			m_move{ move }, m_recommendedDepth{ recommendedDepth },
			m_inAttackSequence{ move.capturedPiece != Piece::None }
		{
		}

		const Move& getMove() const {
			return m_move;
		}
		int getDepth() const {
			return m_recommendedDepth;
		}
		bool inAttackSequence() const {
			return m_inAttackSequence;
		}
	};
}