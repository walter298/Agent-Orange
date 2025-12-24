export module Chess.MoveSearch:Node;

export import Chess.Position;
export import Chess.Position.RepetitionMap;
export import Chess.Rating;
export import Chess.Evaluation;
export import Chess.MoveGeneration;
export import Chess.SafeInt;
export import :MovePriority;

export namespace chess {
	class Node {
	private:
		Position m_pos;
		PositionData m_positionData;

		SafeUnsigned<std::uint8_t> m_level{ 0 };
		SafeUnsigned<std::uint8_t> m_levelsToSearch{ 0 };
		bool m_inAttackSequence = false;
		Rating m_materialExchanged = 0_rt;
		Rating m_materialSignSwap = 1_rt;
		bool m_isChild = true;

		Node() = default;
	public:
		static Node makeRoot(const Position& root, SafeUnsigned<std::uint8_t> maxDepth, bool isWhite) {
			Node ret;
			ret.m_pos = root;
			ret.m_positionData = calcPositionData(ret.m_pos);
			ret.m_levelsToSearch = maxDepth;
			ret.m_isChild = false;
			if (!isWhite) {
				ret.m_materialSignSwap *= -1_rt;
			}

			return ret;
		}
		static Node makeChild(const Node& parent, const MovePriority& movePriority) {
			Node ret;
			ret.m_pos = { parent.m_pos, movePriority.getMove() };
			ret.m_positionData = calcPositionData(ret.m_pos);
			ret.m_level = parent.m_level + 1_su8;
			ret.m_materialSignSwap *= -1_rt;
			ret.m_levelsToSearch = movePriority.getDepth();

			repetition::push(ret.m_pos);

			return ret;
		}
		~Node() {
			if (m_isChild) {
				//std::println("Popping position!");
				repetition::pop();
			}
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

		SafeUnsigned<std::uint8_t> getLevel() const {
			return m_level;
		}

		SafeUnsigned<std::uint8_t>getRemainingDepth() const {
			return m_levelsToSearch;
		}

		bool isDone() const {
			return m_levelsToSearch == 0_su8;
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