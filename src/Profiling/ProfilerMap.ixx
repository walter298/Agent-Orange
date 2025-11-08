export module Chess.Profiler:ProfilerMap;

import std;

import :FunctionProfiler;

namespace chess {
	struct ProfilerNode {
		Profiler* profiler = nullptr;
		std::unordered_set<std::string> childNames;
	};

	void forEachProfilerNode(std::function<void(const std::string&, const ProfilerNode&)> func);
	void mapProfiler(const std::string& name, Profiler* profiler);
	void addChild(const std::string& parent, const std::string& child);
	const ProfilerNode& getProfilerNode(const std::string& name);
}