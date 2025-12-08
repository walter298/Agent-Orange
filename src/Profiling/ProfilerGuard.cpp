module Chess.Profiler:ProfilerGuard;

import nlohmann.json;

import Chess.ProgramEnd;
import :Path;
import :ProfilerMap;
import :Tree;

using namespace std::literals;

namespace chess {
	ProfilerGuard::ProfilerGuard() {
		createTree();
	}

	auto getChildrenPercentages(const ProfilerNode& parent) {
		return parent.childNames | std::views::transform([&](const auto& name) {
			if (parent.profiler->getTotalTimeSpent() == 0ns) {
				return std::pair{ name, 0.0 };
			}
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
				{ "time_spent_ns", node.profiler->getTotalTimeSpent().count() },
				{ "average_ns", node.profiler->getAverage().count() },
				{ "child_percentages", getChildPercentagesJson(node) },
				{ "unique_data", node.profiler->getUniqueJson() }
			});
		});
		
		return json;
	}

	ProfilerGuard::~ProfilerGuard() {
		if (isProgramFinished()) { //program could have ended because of a zAssert
			return;
		}
		markProgramAsFinished();

		auto path = getProfilingSessionFilePath();
		std::ofstream file{ path };
		
		auto j = getGraphJson();
		file << j.dump(2);
	}
}