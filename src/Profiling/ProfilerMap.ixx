export module Chess.Profiler:ProfilerMap;

import std;
import :BasicProfiler;

namespace chess {
	struct ProfilerNode {
		std::deque<BasicProfiler> profilers;
		std::unordered_set<std::string> childNames;

		std::chrono::nanoseconds getTimeSpent() const;
		std::uint64_t getTimesRun() const;
	};

	void forEachProfilerNode(std::function<void(const std::string&, const ProfilerNode&)> func);
	BasicProfiler* addNode(const std::string& childName, const std::string& parentName = "");
	const ProfilerNode& getProfilerNode(const std::string& name);
}