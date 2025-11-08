module Chess.Profiler:ProfilerMap;

namespace chess {
	using namespace std::literals;

	std::unordered_map<std::string, ProfilerNode> profilerMap;

	void forEachProfilerNode(std::function<void(const std::string&, const ProfilerNode&)> func) {
		for (const auto& [name, node] : profilerMap) {
			func(name, node);
		}
	}

	void mapProfiler(const std::string& name, Profiler* profiler) {
		if (!profilerMap.contains(name)) {
			profilerMap.emplace(name, ProfilerNode{ profiler });
		} else {
			throw std::runtime_error{ "Profiler with name "s + name + " already exists." };
		}
	}
	void addChild(const std::string& parentName, const std::string& childName) {
		auto& parentNode = profilerMap.at(parentName);
		if (parentNode.childNames.contains(childName)) {
			throw std::runtime_error{ "Child profiler with name "s + childName + " already exists for parent " + parentName + "." };
		}
		parentNode.childNames.insert(childName);
	}
	const ProfilerNode& getProfilerNode(const std::string& name) {
		return profilerMap.at(name);
	}
}