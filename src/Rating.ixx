export module Chess.Rating;

import std;

export namespace chess {
	using Rating = double;

	constexpr Rating operator"" rt(unsigned long long rating) {
		return static_cast<Rating>(rating);
	}

	constexpr Rating operator"" rt(long double rating) {
		return static_cast<Rating>(rating);
	}

	template<bool Maximizing>
	consteval Rating worstPossibleRating() {
		if constexpr (Maximizing) {
			return std::numeric_limits<Rating>::lowest();
		} else {
			return std::numeric_limits<Rating>::max();
		}
	}

	template<bool Maximizing>
	constexpr Rating checkmatedRating() {
		return std::nextafter(worstPossibleRating<Maximizing>(), 0.0);
	}
}