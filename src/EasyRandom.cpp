module Chess.EasyRandom;

import std;

namespace chess {
	template<std::integral Num>
	Num makeRandomNumberImpl(Num aInc, Num bInc) {
		thread_local std::random_device dev;
		thread_local std::mt19937 rbg{ dev() };
		std::uniform_int_distribution dist{ aInc, bInc };
		return dist(rbg);
	}

	int makeRandomNum(int aInc, int bInc) {
		return makeRandomNumberImpl(aInc, bInc);
	}
	size_t makeRandomNum(size_t aInc, size_t bInc) {
		return makeRandomNumberImpl(aInc, bInc);
	}
}