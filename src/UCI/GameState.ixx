export module Chess.UCI:GameState;

export import std;
import Chess.Position;

namespace chess {
	class GameState {
	private:
		Position m_pos;
		bool m_inNewPos = true;
	public:
		int depth = 6;

		void reset();
		void setPos(const std::string& command);

		std::string calcBestMove();
	};
}