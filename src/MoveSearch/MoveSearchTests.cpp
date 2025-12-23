module Chess.MoveSearch:MoveSearchTests;

import Chess.Position;
import :MoveOrdering;

namespace chess {
	namespace tests {
		void printPriorities(const FixedVector<MovePriority>& priorities) {
			for (const auto& priority : priorities) {
				auto [move, depth] = std::tuple{ priority.getMove(), priority.getDepth() };
				std::println("[{}, {}]", move.getUCIString(), static_cast<unsigned int>(depth.get()));
			}
		}

		void testMoveOrdering() {
			Position pos;
			pos.setPos(parsePositionCommand("fen rnbq1k1r/3p1ppp/1p1b1n1Q/pBp1p3/4P2P/N2P3R/PPP2PP1/R1B1K1N1 b Q - 2 8"));
			auto node = Node::makeRoot(pos, 1_su8, false);
			auto priorities = getMovePriorities(node, Move::null());

			if (priorities[0].getMove().to != H6) {
				std::println("testMoveOrdering failed: queen capture is not the best move");
				printPriorities(priorities);
			}
		}

		void runInternalMoveSearchTests() {
			testMoveOrdering();
		}
	}
}