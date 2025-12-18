export module Chess.MoveSearch:Node;

export import Chess.Position;
export import Chess.Rating;
export import Chess.Evaluation;
export import Chess.LegalMoveGeneration;
export import :MovePriority;

export namespace chess {
	class Node {
	private:
		Position m_pos;
		PositionData m_positionData;

		std::uint8_t m_level = 0;
		std::uint8_t m_levelsToSearch = 0;
		bool m_inAttackSequence = false;
		Rating m_materialExchanged = 0_rt;
		Rating m_materialSignSwap = 1_rt;

		Node() = default;
	public:
		static Node makeRoot(const Position& root, std::uint8_t maxDepth, bool isWhite) {
			Node ret;
			ret.m_pos = root;
			ret.m_positionData = calcPositionData(ret.m_pos);
			ret.m_levelsToSearch = maxDepth;
			if (!isWhite) {
				ret.m_materialSignSwap *= -1_rt;
			}

			return ret;
		}
		static Node makeChild(const Node& parent, const MovePriority& movePriority) {
			Node ret;
			ret.m_pos = { parent.m_pos, movePriority.getMove() };
			ret.m_positionData = calcPositionData(ret.m_pos);
			ret.m_level = parent.m_level + 1;
			if (movePriority.inAttackSequence()) {
				ret.m_inAttackSequence = true;
				if (movePriority.getMove().capturedPiece != Piece::None) {
					ret.m_materialExchanged += (getPieceRating(movePriority.getMove().capturedPiece) * parent.m_materialSignSwap);
				}
			}
			ret.m_materialSignSwap *= -1_rt;
			ret.m_levelsToSearch = movePriority.getDepth();
			return ret;
		}

		bool inAttackSequence() const {
			return m_inAttackSequence;
		}

		bool inWinningAttackSequence() const {
			return m_materialSignSwap > 0_rt ? m_materialExchanged > 0_rt : m_materialExchanged < 0_rt;
		}

		bool inLosingAttackSequence() const {
			return m_materialSignSwap > 0_rt ? m_materialExchanged < 0_rt : m_materialExchanged > 0_rt;
		}

		const Position& getPos() const {
			return m_pos;
		}
		const PositionData& getPositionData() const {
			return m_positionData;
		}

		std::uint8_t getLevel() const {
			return m_level;
		}

		std::uint8_t getRemainingDepth() const {
			return m_levelsToSearch;
		}

		bool isDone() const {
			return m_levelsToSearch == 0;
		}

		Rating getRating() const {
			return staticEvaluation(m_pos, m_positionData);
		}

		Bitboard getEnemySquares() const {
			return m_pos.isWhite() ? m_positionData.blackSquares : m_positionData.whiteSquares;
		}

		const PieceState& getAllies() const {
			auto [white, black] = m_pos.getColorSides();
			return m_pos.isWhite() ? white : black;
		}
	};
}