import json
import os

from pie_chart import *
from stats import Stats

def parse_pie_chart(child_percentages_json) -> PieChart:
    slices = []
    if not child_percentages_json:
        return None
    
    for name, percentage in child_percentages_json.items():
        slices.append(Slice(name, float(percentage)))

    return PieChart(slices)

def load_stats_file(file_path) -> list[Stats]:
    stats = []
    with open(file_path, 'r') as f:
        stats_json = json.load(f)
        for func in stats_json:
            average = int(func["average_ns"])
            name = func["name"]
            pie_chart = parse_pie_chart(func["child_percentages"])
            stats.append(Stats(name, average, pie_chart))
        return stats
    
def load_stats_map() -> dict[(str, list[Stats])]:
    dir_path = os.environ.get("CHESS_PROFILING_SESSIONS")
    all_stats = {}
    for file_name in os.listdir(dir_path):
        if file_name.endswith('.json'):
            session_name = file_name[:-5]  #remove .json extension
            file_path = os.path.join(dir_path, file_name)
            all_stats[session_name] = load_stats_file(file_path)
    return all_stats

if __name__ == "__main__":
    loaded_stats = load_stats_map()
    for session_name, stats in loaded_stats:
        print(f"Session: {session_name}")
        for stat in stats:
            print(f"Function: {stat.name()}, Average: {stat.average()}, Percentage: {stat.percentage()}")