export module Chess.UCI:GameState;

export import std;

import Chess.Position;
import Chess.Position.RepetitionMap;
import Chess.MoveSearch;
import Chess.SafeInt;

namespace chess {
	struct GameState {
		Position pos;
		SafeUnsigned<std::uint8_t> depth{ 6_su8 };
		RepetitionMap repetitionMap;
	};
	class SearchThread {
	private:
		std::mutex m_mutex;
		std::condition_variable_any m_cv;
		AsyncSearch m_searcher;
		GameState m_state;
		bool m_calculationRequested = false;
		std::jthread m_thread; //thread is destroyed before all other members
		
		void run(std::stop_token stopToken);
	public:
		SearchThread(SafeUnsigned<std::uint8_t> depth);
		~SearchThread();
		void stop();
		void go(GameState gameState);
	};

	class Engine {
	private:
		SearchThread m_searchThread;
		GameState m_state;
	public:
		Engine(SafeUnsigned<std::uint8_t> depth);

		void setPos(const std::string& command);

		void printBestMoveAsync(SafeUnsigned<std::uint8_t> depth = 6_su8);
		void stopCalculating();
	};
}