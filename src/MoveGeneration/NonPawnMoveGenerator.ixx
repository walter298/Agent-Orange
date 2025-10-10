export module Chess.LegalMoveGeneration:NonPawnMoveGenerator;

import :KingMoveGeneration;
import :KnightMoveGeneration;
import :SlidingMoveGeneration;
import :ChainedMoveGenerator;

export namespace chess {
	template <typename, template <typename...> class>
	struct IsTemplate : std::false_type {};

	template<template<typename...> class Template, typename... Args>
	struct IsTemplate<Template<Args...>, Template> : std::true_type {};

	template<typename T>
	concept NonPawnMoveGenerator = 
		std::same_as<KingMoveGenerator, T> ||
		std::same_as<OrthogonalMoveGenerator, T> ||
		std::same_as<DiagonalMoveGenerator, T> ||
		std::same_as<KnightMoveGenerator, T> ||
		IsTemplate<T, ChainedMoveGenerator>::value;
}