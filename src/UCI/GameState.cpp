module;

#include <cassert>

module Chess.UCI:GameState;

import Chess.MoveSearch;
import Chess.PositionCommand;

namespace chess {
	void GameState::reset() {
		m_inNewPos = true;
	}

	void GameState::setPos(const std::string& commandStr) {
		auto command = parsePositionCommand(commandStr);
		if (m_inNewPos) {
			m_pos.setPos(command);
		} else {
			assert(!command.moves.empty());
			m_pos.move(command.moves.back());
		}
		m_inNewPos = false;
	}

	std::string GameState::calcBestMove() {
		auto move = findBestMove(m_pos, 6); 
		if (!move) {
			return "";
		} else {
			m_pos.move(*move);
			return move->getUCIString();
		}
	}
}