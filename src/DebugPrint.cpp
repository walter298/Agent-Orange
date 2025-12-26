module Chess.DebugPrint;

import Chess.EnvironmentVariable;

namespace chess {
	class Buffer {
	private:
		std::unordered_map<std::thread::id, std::string> m_map;
		std::mutex m_mutex;
	public:
		void add(const std::string& s) {
			std::lock_guard l{ m_mutex };
			m_map[std::this_thread::get_id()] += s + '\n';
		}
		~Buffer() {
			std::ofstream file{ getAssetDirectoryPath() / "debug_output.txt" };
			for (const auto& [id, text] : m_map) {
				file << std::format("Thread [{}]: \n", id);
				file << text << '\n';
			}
		}
	};
	Buffer buff;

	void debugPrint(const std::string& s) {
		std::println("{}", s);
		buff.add(s);
	}
}