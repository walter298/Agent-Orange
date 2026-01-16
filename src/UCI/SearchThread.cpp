module;

#include <cstdio>

module Chess.UCI:SearchThread;

import Chess.Assert;
import Chess.DebugPrint;
import Chess.MoveSearch;
import Chess.PositionCommand;
import Chess.Position.RepetitionMap;

using namespace std::literals;

namespace chess {
	void SearchThread::think(std::stop_token stopToken) {
		while (!stopToken.stop_requested()) {
			{
				std::scoped_lock l{ m_mutex };
				if (m_calculationRequested) {
					break;
				}
			}

			std::unique_lock ul{ m_mutex };
			m_cv.wait(ul, stopToken, [this] {
				return m_shouldPonder || m_calculationRequested;
			});

			if (stopToken.stop_requested() || !m_shouldPonder || m_calculationRequested) {
				break;
			}

			auto stateCopy = m_state;
			ul.unlock();

			std::println("Thinking on your time Mr. Anderson...");

			auto move = m_searcher.findBestMove(stateCopy.pos, 255_su8, stateCopy.repetitionMap);
			if (!move) { //if the position has no legal moves, then reset to an invalid position state
				std::scoped_lock l{ m_mutex };
				if (!m_calculationRequested) { //ensure that we don't flag that we are not in a valid position after a new calculation request
					m_shouldPonder = false;
				}
			}
		}
	}

	void SearchThread::run(std::stop_token stopToken) {
		using namespace std::literals;

		while (!stopToken.stop_requested()) {
			think(stopToken);

			GameState stateCopy;
			{
				std::unique_lock l{ m_mutex };
				stateCopy = m_state;
				m_calculationRequested = false;
			}

			if (auto bestMove = m_searcher.findBestMove(stateCopy.pos, stateCopy.depth, stateCopy.repetitionMap)) {
				if (!stopToken.stop_requested()) {
					std::println("{}", bestMove->getUCIString());
					std::fflush(stdout);
					m_state.pos.move(*bestMove);
					std::scoped_lock l{ m_mutex };
					m_shouldPonder = true;
				}
			} else { //GUI sent us a position with zero legal moves
				std::scoped_lock l{ m_mutex };
				m_shouldPonder = false;
			}
		}
	}

	SearchThread::SearchThread(SafeUnsigned<std::uint8_t> depth)
		: m_searcher{ depth }
	{
		m_thread = std::jthread{ [this](std::stop_token stopToken){ run(stopToken); } };
	}
	SearchThread::~SearchThread() {
		m_searcher.cancel(); //in case we are stuck in findBestMove
	}

	void SearchThread::setPosition(GameState state) {
		{
			std::scoped_lock l{ m_mutex };
			m_shouldPonder = true;
			m_state = std::move(state);
		}
		m_cv.notify_one();
	}

	void SearchThread::go() {
		{
			std::scoped_lock l{ m_mutex };
			m_calculationRequested = true;
			m_shouldPonder = false;
		}
		m_searcher.cancel();
		m_cv.notify_one();
	}

	void SearchThread::stop() { 
		m_searcher.cancel(); //internally synchronized
		{
			std::scoped_lock l{ m_mutex };
			m_shouldPonder = false;
		}
		m_cv.notify_one();
	}
}