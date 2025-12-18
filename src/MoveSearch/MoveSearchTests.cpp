module Chess.MoveSearch:MoveSearchTests;

import Chess.Position;
import :MoveOrdering;

namespace chess {
	namespace tests {
		void printPriorities(const FixedVector<MovePriority>& priorities) {
			for (const auto& priority : priorities) {
				auto [move, depth] = std::tuple{ priority.getMove(), priority.getDepth() };
				std::println("[{}, {}]", move.getUCIString(), static_cast<int>(depth));
			}
		}

		void testMoveOrdering() {
			Position pos;
			pos.setPos(parsePositionCommand("fen rnb1kbnr/pp1pppp1/2p5/q6p/2PPP3/8/PP1B1PPP/RN1QKBNR b KQkq - 1 1"));
			auto node = Node::makeRoot(pos, 1, false);
			auto priorities = getMovePriorities(node, Move::null());
			auto queenSacIt = std::ranges::find_if(priorities, [](const auto& priority) {
				return priority.getMove().from == A5 && priority.getMove().to == B4;
			});
			if (queenSacIt != priorities.end()) {
				std::println("testMoveOrdering failed: queen sacrifice contained in move priorities");
				printPriorities(priorities);
			}
		}

		void runInternalMoveSearchTests() {
			testMoveOrdering();
		}
	}
}