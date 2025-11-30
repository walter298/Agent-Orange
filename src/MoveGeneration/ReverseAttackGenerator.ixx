module;

#define MAKE_OPPOSITE_DIRECTIONS(dir1, dir2) \
	template<>\
	struct ReverseAttackGenerator<std::remove_cvref_t<decltype(dir1)>> {\
		static consteval auto get() { return dir2; }\
	}; \
	template<>\
	struct ReverseAttackGenerator<std::remove_cvref_t<decltype(dir2)>> { \
		static consteval auto get() { return dir1; }\
	}; \

#define MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(dir) \
	template<>\
	struct ReverseAttackGenerator<std::remove_cvref_t<decltype(dir)>> {\
		static consteval auto get() { return dir; }\
	}; \

export module Chess.LegalMoveGeneration:ReverseAttackGenerator;

import std;
export import :KnightMoveGeneration;
export import :PawnMoveGeneration;
export import :SlidingMoveGeneration;

export namespace chess {
	template<typename T>
	struct ReverseAttackGenerator {}; //primary template

	MAKE_OPPOSITE_DIRECTIONS(whitePawnAttackGenerator, blackPawnAttackGenerator)
	MAKE_OPPOSITE_DIRECTIONS(northWestSlidingMoveGenerator, southEastSlidingMoveGenerator)
	MAKE_OPPOSITE_DIRECTIONS(northEastSlidingMoveGenerator, southWestSlidingMoveGenerator)
	MAKE_OPPOSITE_DIRECTIONS(northSlidingMoveGenerator, southSlidingMoveGenerator)
	MAKE_OPPOSITE_DIRECTIONS(eastSlidingMoveGenerator, westSlidingMoveGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(diagonalMoveGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(orthogonalMoveGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(diagonalMoveGenerator | orthogonalMoveGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(knightMoveGenerator)
}