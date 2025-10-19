export module Chess.UCI:GameState;

export import std;
import Chess.Position;

namespace chess {
	class GameState {
	private:
		Position m_pos;
		bool m_inNewPos = true;
	public:
		void reset();
		void setPos(std::string_view fen, const std::vector<std::string>& moves);
		
		std::string calcBestMove();
	};
}