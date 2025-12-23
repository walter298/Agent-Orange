module Chess.MoveGeneration:RayTable;

import nlohmann.json;

import Chess.EnvironmentVariable;
import Chess.Square;
import :BruteForceDirectionalMoveGeneration;

namespace chess {
	using Table = SquareMap<SquareMap<Bitboard>>;

	Bitboard makeRay(Square s1, Square s2) {
		auto from = makeBitboard(s1);
		auto empty = ~makeBitboard(s2);
		
		constexpr std::tuple ALL_DIRECTION_GENERATORS{
			northEastSlidingMoveGenerator,
			northWestSlidingMoveGenerator,
			northSlidingMoveGenerator,
			westSlidingMoveGenerator,
			eastSlidingMoveGenerator,
			southEastSlidingMoveGenerator,
			southWestSlidingMoveGenerator,
			southSlidingMoveGenerator
		};

		auto getRay = [&](auto gen) -> Bitboard {
			auto destSquares = gen(from, empty);
			if (destSquares.nonEmptyDestSquares) { //we hit the square!
				return destSquares.emptyDestSquares;
			}
			return 0_bb;
		};
		return std::apply([&](auto... generators) {
			return (... | getRay(generators));
		}, ALL_DIRECTION_GENERATORS);
	}

	Table generateRayTable() {
		Table table;

		constexpr auto SQUARE_PAIRS = std::views::cartesian_product(SQUARE_ARRAY, SQUARE_ARRAY);
		for (const auto& [s1, s2] : SQUARE_PAIRS) {
			table[s1][s2] = makeRay(s1, s2);
		}
		return table;
	}

	Bitboard getRay(Square from, Square to) {
		static const auto table = generateRayTable();
		return table[from][to];
	}
}