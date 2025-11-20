module Chess.Profiler:ProfilerMap;

namespace chess {
	using namespace std::literals;

	std::unordered_map<std::string, ProfilerNode> profilerMap;

	void forEachProfilerNode(std::function<void(const std::string&, const ProfilerNode&)> func) {
		for (const auto& [name, node] : profilerMap) {
			func(name, node);
		}
	}

	BasicProfiler* mapProfiler(const std::string& name) {
		return &profilerMap[name].profiler;
	}

	void addChild(const std::string& parentName, const std::string& childName) {
		auto& parentNode = profilerMap.at(parentName);
		parentNode.childNames.insert(childName);
	}

	const ProfilerNode& getProfilerNode(const std::string& name) {
		return profilerMap.at(name);
	}
}