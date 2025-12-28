module Chess.UCI:GameState;

import Chess.Assert;
import Chess.DebugPrint;
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
			repetition::push(m_pos);
		} else { //receiving a new move in the same position
			zAssert(!command.moves.empty());
			m_pos.move(command.moves.back());
			repetition::push(m_pos);
		}
		m_inNewPos = false;
	}

	std::string GameState::calcBestMove() {
		auto move = findBestMove(m_pos, depth); 
		if (!move) {
			return "";
		} else {
			m_pos.move(*move);
			repetition::push(m_pos);
			return move->getUCIString();
		}
	}
}