module Chess.UCI:GameState;

import Chess.Assert;
import Chess.MoveSearch;
import Chess.PositionCommand;
import Chess.Position.RepetitionMap;

namespace chess {
	void GameState::reset() {
		m_inNewPos = true;
		repetition::clear();
	}

	void GameState::setPos(const std::string& commandStr) {
		auto command = parsePositionCommand(commandStr);
		if (m_inNewPos) {
			m_pos.setPos(command);
		} else {
			zAssert(!command.moves.empty());
			m_pos.move(command.moves.back());
		}
		m_inNewPos = false;
	}

	std::string GameState::calcBestMove() {
		auto move = findBestMove(m_pos, depth); 
		if (!move) {
			return "";
		} else {
			m_pos.move(*move);
			return move->getUCIString();
		}
	}
}