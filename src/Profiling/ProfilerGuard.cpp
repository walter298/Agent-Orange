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
			if (parent.getAverageTimeSpent() == 0ns) {
				return std::pair{ name, 0.0 };
			}
			const auto& child = getProfilerNode(name);
			auto percentage = static_cast<double>(child.getAverageTotalTimeSpentAcrossThreads().count()) /
							  static_cast<double>(parent.getAverageTotalTimeSpentAcrossThreads().count());
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
			auto timesRun = node.getTimesRun();
			auto averageTimeSpent = node.getAverageTimeSpent();
			
			json.push_back({
				{ "name", name },
				{ "times_run", timesRun },
				{ "average_ns", averageTimeSpent.count() },
				{ "child_percentages", getChildPercentagesJson(node) }
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