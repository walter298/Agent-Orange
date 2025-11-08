module;

#include <cassert>
#include <stdlib.h>

module Chess.Profiler:Path;

namespace chess {
	std::filesystem::path getProfilingSessionsDirectoryPath() {
		constexpr auto MAX_ENV_LEN = 256;
		std::array<char, MAX_ENV_LEN> buff;

		auto envLen = 0uz;
		auto res = getenv_s(&envLen, buff.data(), MAX_ENV_LEN, "CHESS_PROFILING_SESSIONS");
		if (res != 0 || envLen == 0) {
			std::println("Error getting CHESS_PROFILING_SESSIONS environment variable.");
			std::exit(EXIT_FAILURE);
		}
		return std::filesystem::path{ std::string_view{ buff.data(), envLen - 1 } }; //subtract 1 for null terminator
	}

	std::filesystem::path getProfilingSessionFilePath() {
		auto dir = getProfilingSessionsDirectoryPath();
		assert(std::filesystem::exists(dir));

		auto now = std::chrono::system_clock::now();
		auto timeStamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);

		std::ranges::replace(timeStamp, ':', '-');
		std::ranges::replace(timeStamp, ' ', '_');

		return dir / (timeStamp + ".json");
	}
}
