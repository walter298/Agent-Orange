export module Chess.UCI:GameState;

export import std;

import Chess.Position;
import Chess.Position.RepetitionMap;
import Chess.SafeInt;

namespace chess {
	class GameState {
	private:
		Position m_pos;
		RepetitionMap m_repetitionMap;
		bool m_inNewPos = true;
	public:
		SafeUnsigned<std::uint8_t> depth{ 6 };

		void reset();
		void setPos(const std::string& command);

		std::string calcBestMove();
	};
}