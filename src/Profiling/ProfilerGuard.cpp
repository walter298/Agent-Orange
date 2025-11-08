module;

#include <nlohmann/json.hpp>

module Chess.Profiler:ProfilerGuard;

import :FunctionProfiler;
import :Path;
import :ProfilerMap;

namespace chess {
	auto getChildrenPercentages(const ProfilerNode& parent) {
		return parent.childNames | std::views::transform([&](const auto& name) {
			auto& child = getProfilerNode(name);
			auto percentage = static_cast<double>(child.profiler->getTotalTimeSpent().count()) /
							  static_cast<double>(parent.profiler->getTotalTimeSpent().count());
			percentage *= 100.0;
			return std::pair{ name, percentage };
		});
	}

	nlohmann::json getChildPercentagesJson(const ProfilerNode& parent) {
		nlohmann::json ret;
		auto childrenPercentages = getChildrenPercentages(parent);
		for (const auto& [childName, percentage] : childrenPercentages) {
			ret[childName] = percentage;
		}
		return ret;
	}

	nlohmann::json getGraphJson() {
		auto json = nlohmann::json::array();
		forEachProfilerNode([&](const std::string& name, const ProfilerNode& node) {
			json.push_back({
				{ "name", name },
				{ "times_run", node.profiler->getTimesRun() },
				{ "average_ns", std::format("{}", node.profiler->getAverage().count()) },
				{ "child_percentages", getChildPercentagesJson(node) }
			});
		});
		
		return json;
	}

	ProfilerGuard::~ProfilerGuard() {
		auto path = getProfilingSessionFilePath();
		std::ofstream file{ path };
		assert(file.is_open());

		auto j = getGraphJson();
		file << j.dump(2);

		assert(std::filesystem::exists(path));
	}
}