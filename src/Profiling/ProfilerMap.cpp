module Chess.Profiler:ProfilerMap;

import :BasicProfiler;

namespace chess {
	using namespace std::literals;

	std::chrono::nanoseconds ProfilerNode::getTimeSpent() const {
		return std::ranges::fold_left(profilers, 0ns, [](auto acc, const auto& profiler) {
			return acc + profiler.getTotalTimeSpent();
		});
	}
	std::uint64_t ProfilerNode::getTimesRun() const {
		return std::ranges::fold_left(profilers, 0, [](auto acc, const auto& profiler) {
			return acc + profiler.getTimesRun();
		});
	}

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

	BasicProfiler* addParent(const std::string& name) {
		if (getProfilerMap().contains(name)) {
			throw std::runtime_error{ "Error: Profiler with name '"s + name + "' is already mapped."s };
		}
		return &(getProfilerMap()[name].profilers.emplace_back());
	}

	BasicProfiler* addNode(const std::string& childName, const std::string& parentName) {
		static std::mutex mutex;
		std::lock_guard l{ mutex };

		auto profiler = &(getProfilerMap()[childName].profilers.emplace_back());

		if (!parentName.empty()) {
			auto& parentNode = getProfilerMap().at(parentName);
			parentNode.childNames.insert(childName);
		}
		
		return profiler;
	}

	const ProfilerNode& getProfilerNode(const std::string& name) {
		return getProfilerMap().at(name);
	}
}