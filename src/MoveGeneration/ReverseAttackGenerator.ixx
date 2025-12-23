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

export module Chess.MoveGeneration:ReverseAttackGenerator;

import std;
export import :KnightMoveGeneration;
export import :PawnMoveGeneration;
export import :SlidingMoveGenerators;

export namespace chess {
	template<typename T>
	struct ReverseAttackGenerator {}; //primary template

	MAKE_OPPOSITE_DIRECTIONS(whitePawnAttackGenerator, blackPawnAttackGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(knightMoveGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(bishopMoveGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(rookMoveGenerator)
	MAKE_SYMMETRICAL_OPPOSITE_DIRECTION(queenMoveGenerator)
}