module Chess.Profiler:Path;

import Chess.Assert;
import Chess.EnvironmentVariable;

namespace chess {
	std::filesystem::path getProfilingSessionFilePath() {
		auto dir = getAssetDirectoryPath() / "profiling_sessions";
		zAssert(std::filesystem::exists(dir));

		auto now = std::chrono::system_clock::now();
		auto timeStamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);

		std::ranges::replace(timeStamp, ':', '-');
		std::ranges::replace(timeStamp, ' ', '_');

		return dir / (timeStamp + ".json");
	}
}
