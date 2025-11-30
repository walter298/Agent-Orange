export module Chess.LegalMoveGeneration:NonPawnMoveGenerator;

import :KingMoveGeneration;
import :KnightMoveGeneration;
import :SlidingMoveGeneration;
import :ChainedMoveGenerator;
import :PawnMoveGeneration;

export namespace chess {
	template<typename, template<typename...> typename>
	struct IsTemplate : std::false_type {};

	template<template<typename...> class Template, typename... Args>
	struct IsTemplate<Template<Args...>, Template> : std::true_type {};

	template<typename T>
	concept PawnMoveGenerator = std::same_as<std::remove_cvref<T>, WhitePawnAttackGenerator> ||
								std::same_as<std::remove_cvref<T>, WhitePawnMoveGenerator> ||
								std::same_as<std::remove_cvref<T>, BlackPawnAttackGenerator> ||
								std::same_as<std::remove_cvref<T>, BlackPawnMoveGenerator>;
}