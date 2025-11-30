module;

#include <cassert>
#include <cstdlib>

module Chess.Profiler:Path;

import Chess.EnvironmentVariable;

namespace chess {
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
