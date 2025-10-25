module;

#include <cassert>

module Chess.UCI:GameState;

import Chess.Evaluation;

namespace chess {
	void GameState::reset() {
		m_inNewPos = true;
	}

	void GameState::setPos(std::string_view fen, const std::vector<std::string>& moves) {
		if (m_inNewPos) {
			m_pos.setFen(fen);
			for (const auto& move : moves) {
				m_pos.move(move);
			}
		} else {
			assert(!moves.empty());
			m_pos.move(moves.back());
		}
		m_inNewPos = false;
	}

	std::string GameState::calcBestMove() {
		auto move = bestMove(m_pos, 2); //todo: handle depth
		if (!move) {
			return "";
		} else {
			m_pos.move(*move);
			return move->getUCIString();
		}
	}
}