export module Chess.Position:PositionData;

export import std;

export import Chess.Bitboard;
export import Chess.Move;
export import Chess.MoveGen;
export import Chess.PieceMap;
export import Chess.Rating;
export import Chess.Position.PieceState;

export namespace chess {
	using MoveVector = std::vector<Move>;

	struct PositionData {
	private:
		PieceMap<MoveGen>* m_allySquares = nullptr;
		PieceMap<MoveGen>* m_enemySquares = nullptr;
	public:
		MoveVector legalMoves;
		PieceMap<MoveGen> whiteSquares;
		PieceMap<MoveGen> blackSquares;
		
		bool isCheck = false;
		
		constexpr PositionData(bool isWhite) {
			if (isWhite) {
				m_allySquares = &whiteSquares;
				m_enemySquares = &blackSquares;
			} else {
				m_allySquares = &blackSquares;
				m_enemySquares = &whiteSquares;
			}
		}
		auto& getAllySquares(this auto&& self) {
			return *self.m_allySquares;
		}
		auto& getEnemySquares(this auto&& self) {
			return *self.m_enemySquares;
		}
	
		static Bitboard allSquares(const PieceMap<MoveGen>& squares) {
			return std::ranges::fold_left(squares, 0_bb, [](auto acc, const MoveGen& moveGen) {
				return acc | moveGen.all();
			});
		}
	
		Bitboard allAllySquares() const {
			return allSquares(*m_allySquares);
		}
		Bitboard allEnemySquares() const {
			return allSquares(*m_enemySquares);
		}
		bool isCheckmate() const {
			return legalMoves.empty() && isCheck;
		}
	};
}