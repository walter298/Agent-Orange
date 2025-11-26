module Chess.Profiler:ProfilerMap;

namespace chess {
	using namespace std::literals;

	using ProfilerMap = std::unordered_map<std::string, ProfilerNode>;

	ProfilerMap& getProfilerMap() {
		static ProfilerMap map;
		return map;
	}
	
	void forEachProfilerNode(std::function<void(const std::string&, const ProfilerNode&)> func) {
		for (const auto& [name, node] : getProfilerMap()) {
			func(name, node);
		}
	}

	void mapProfiler(const std::string& name, BasicProfiler* profiler) {
		if (getProfilerMap().contains(name)) {
			throw std::runtime_error{ "Error: Profiler with name '"s + name + "' is already mapped."s };
		}
		getProfilerMap().emplace(name, ProfilerNode{ profiler, {} });
	}

	void addChild(const std::string& childName, const std::string& parentName) {
		auto& parentNode = getProfilerMap().at(parentName);
		parentNode.childNames.insert(childName);
	}

	const ProfilerNode& getProfilerNode(const std::string& name) {
		return getProfilerMap().at(name);
	}
}