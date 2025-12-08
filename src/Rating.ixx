export module Chess.Rating;

import std;

export namespace chess {
	using Rating = float;

	constexpr Rating operator"" _rt(unsigned long long rating) {
		return static_cast<Rating>(rating);
	}

	constexpr Rating operator"" _rt(long double rating) {
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

	auto nextAfter(Rating r) {
		if constexpr (std::same_as<Rating, float>) {
			return std::nextafterf(r, 0_rt);
		} else {
			return std::nextafter(r, 0_rt);
		}
	}

	template<bool Maximizing>
	constexpr Rating checkmatedRating() {
		return nextAfter(worstPossibleRating<Maximizing>());
	}
}