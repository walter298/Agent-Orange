export module Chess.Profiler:ProfilerMap;

import std;

import :BasicProfiler;

namespace chess {
	struct ProfilerNode {
		BasicProfiler* profiler = nullptr;
		std::unordered_set<std::string> childNames;
	};

	void forEachProfilerNode(std::function<void(const std::string&, const ProfilerNode&)> func);
	void mapProfiler(const std::string& name, BasicProfiler* profiler);
	void addChild(const std::string& childName, const std::string& parentName);
	const ProfilerNode& getProfilerNode(const std::string& name);
}