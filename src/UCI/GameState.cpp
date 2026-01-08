module;

#include <cstdio>

module Chess.UCI:GameState;

import Chess.Assert;
import Chess.DebugPrint;
import Chess.MoveSearch;
import Chess.PositionCommand;
import Chess.Position.RepetitionMap;

namespace chess {
	void SearchThread::run(std::stop_token stopToken) {
		using namespace std::literals;

		while (!stopToken.stop_requested()) {
			GameState stateCopy;
			{
				std::unique_lock l{ m_mutex };
				m_cv.wait(l, stopToken, [this] {
					return m_calculationRequested;
				});
				if (stopToken.stop_requested()) {
					break;
				}
				stateCopy = m_state;
				m_calculationRequested = false;
			}

			if (auto bestMove = m_searcher.findBestMove(stateCopy.pos, stateCopy.depth, stateCopy.repetitionMap)) {
				if (!stopToken.stop_requested()) {
					std::println("{}", bestMove->getUCIString());
					std::fflush(stdout);
				}
			}
		}
	}

	SearchThread::SearchThread() {
		m_thread = std::jthread{ [this](std::stop_token stopToken){ run(stopToken); } };
	}
	SearchThread::~SearchThread() {
		m_searcher.cancel(); //in case we are stuck in findBestMove
		m_cv.notify_one();
	}

	void SearchThread::go(GameState state) {
		{
			std::scoped_lock l{ m_mutex };
			m_state = std::move(state);
			m_calculationRequested = true;
		}
		m_cv.notify_one();
	}

	void SearchThread::stop() { 
		m_searcher.cancel(); //internally synchronized
	}

	void Engine::setPos(const std::string& commandStr) {
		m_state.repetitionMap.clear();

		auto command = parsePositionCommand(commandStr);
		m_state.pos.setPos(command);
		m_state.repetitionMap.push(m_state.pos);

		for (const auto& move : command.moves) {
			m_state.pos.move(move);
			m_state.repetitionMap.push(m_state.pos);
		}
	}

	void Engine::printBestMoveAsync(SafeUnsigned<std::uint8_t> depth) {
		m_state.depth = depth;
		m_searchThread.go(m_state);
	}

	void Engine::stopCalculating() {
		m_searchThread.stop();
	}
}