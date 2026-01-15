module Chess.DebugPrint;

import Chess.EnvironmentVariable;

namespace chess {
	class Buffer {
	private:
		std::unordered_map<std::thread::id, std::string> m_map;
		std::mutex m_mutex;
	public:
		void add(const std::string& s) {
			std::scoped_lock l{ m_mutex };
			m_map[std::this_thread::get_id()] += s + '\n';
		}
		void flush() {
			std::scoped_lock l{ m_mutex };
			std::ofstream file{ getAssetDirectoryPath() / "debug_output.txt" };
			for (const auto& [id, text] : m_map) {
				file << std::format("Thread [{}]: \n", id);
				file << text << '\n';
			}
		}
		~Buffer() {
			flush();
		}
	};
	Buffer buff;

	void debugPrint(const std::string& s) {
		buff.add(s);
	}

	void debugPrintFlush() {
		buff.flush();
	}
}