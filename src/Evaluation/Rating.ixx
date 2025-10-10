export module Chess.Evaluation:Rating;

export namespace chess {
	using Rating = double;

	constexpr Rating MAX_WHITE = 100.0;
	constexpr Rating MAX_BLACK = -100.0;
}