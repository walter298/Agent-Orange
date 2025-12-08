export module Chess.Assert;

import std;

export namespace chess {
	void zAssert(bool condition, std::source_location _sl = std::source_location::current()) {
		if constexpr (USING_ASSERT) {
			if (!condition) {
				std::println("Assert failed: file {}, line {}", _sl.file_name(), _sl.line());
				std::abort();
			}
		}
	}
}