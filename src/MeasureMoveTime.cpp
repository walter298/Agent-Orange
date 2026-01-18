module Chess.MeasureMoveTime;

import std;
import nlohmann.json;

import Chess.EnvironmentVariable;
import Chess.Position;
import Chess.Position.RepetitionMap;
import Chess.PositionCommand;
import Chess.MoveSearch;
import Chess.SafeInt;

namespace std {
	void to_json(nlohmann::json& j, const chrono::nanoseconds& ns) {
		j = ns.count();
	}
	void from_json(const nlohmann::json& j, chrono::nanoseconds& ns) {
		ns = chrono::nanoseconds{ j.get<std::chrono::nanoseconds::rep>() };
	}
}

namespace chess {
	std::vector<Position> getPositionInputs() {
		constexpr auto POSITION_COUNT = 3uz;
		const std::array<std::string, POSITION_COUNT> POSITION_INPUTS = {
			"fen rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
			"fen rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
			"fen rnbqkbnr/pppppppp/8/8/3P4/8/PPP1PPPP/RNBQKBNR b KQkq - 0 1"
		};
		std::vector<Position> positions(POSITION_COUNT);
		for (const auto& [pos, fen] : std::views::zip(positions, POSITION_INPUTS)) {
			pos.setPos(parsePositionCommand(fen));
		}
		return positions;
	}

	using namespace std::literals;

	constexpr auto MAX_DEPTH = 8;

	using AverageArray = std::array<std::chrono::nanoseconds, 32>; //average time with x pieces on the board
	using DepthArray   = std::array<AverageArray, MAX_DEPTH - 1>; //average time with k depth and x pieces on the board

	size_t calcPieceCount(const Position& pos) {
		auto [white, black] = pos.getColorSides();
		auto whitePieces = white.calcAllLocations();
		auto blackPieces = black.calcAllLocations();
		return static_cast<size_t>(std::popcount(whitePieces) + std::popcount(blackPieces));
	}

	void measureMoveTimeImpl(AverageArray& averageArray, AsyncSearch& search, SafeUnsigned<std::uint8_t> depth, const Position& startPos) {
		clearTranspositionTable();

		std::ranges::fill(averageArray, 0ns);

		auto pos = startPos;

		RepetitionMap repetitionMap;
		repetitionMap.push(pos);

		std::array<int, 32> moveCountsWithNPieces;
		std::ranges::fill(moveCountsWithNPieces, 0);

		while (true) {
			auto pieceCount = calcPieceCount(pos);

			auto start = std::chrono::steady_clock::now();
			auto bestMove = search.findBestMove(pos, depth, repetitionMap);
			if (!bestMove) {
				break;
			}

			//update the position
			pos.move(*bestMove);
			repetitionMap.push(pos);

			auto end = std::chrono::steady_clock::now();

			auto timeSpent = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
			averageArray[pieceCount - 1] += timeSpent;
			moveCountsWithNPieces[pieceCount - 1]++;
		}

		for (auto [average, moveCount] : std::views::zip(averageArray, moveCountsWithNPieces)) {
			if (moveCount > 0) {
				average /= moveCount;
			}
		}
	}

	void calcAverageTimeDecrease(const AverageArray& averageArray, SafeUnsigned<std::uint8_t> depth) {
		auto ret = 0ns;

		auto reversePairs = std::views::reverse(averageArray) | std::views::adjacent<2>;
		int pieceCount = 32;

		for (const auto [t1, t2] : reversePairs) {
			auto change = t1 - t2;
			std::println("Change from {} to {} pieces at depth {}: {}", pieceCount, pieceCount - 1, static_cast<std::uint32_t>(depth.get()),change);
			pieceCount--;
		}
	}

	void measureMoveTime() {
		AsyncSearch search;
		DepthArray depthArray{};

		auto positions = getPositionInputs();
		for (auto depth = 1_su8; depth < SafeUnsigned<std::uint8_t>{ MAX_DEPTH }; ++depth) {
			std::println("Measuring depth {}", static_cast<std::uint32_t>(depth.get()));

			auto& averageArray = depthArray[(depth - 1_su8).get()];
			for (const auto& [i, pos]: std::views::enumerate(positions)) {
				std::println("[{}]...", i);
				measureMoveTimeImpl(averageArray, search, depth, pos);
			}
			
			calcAverageTimeDecrease(averageArray, depth);
		}

		nlohmann::json j = depthArray;
		std::ofstream file{ getAssetDirectoryPath() / "depth_time.json" };
		file << j.dump(2);
	}
}