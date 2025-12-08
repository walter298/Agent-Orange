module;

#ifdef _WIN64
#include <Windows.h>
#endif

module Chess.Tests:Pipe;

namespace chess {
	constexpr size_t READ_BUFF_MAX_SIZE = 8196;
	using ReadBuffer = std::array<char, READ_BUFF_MAX_SIZE>;

	template<typename T>
	concept Reader = std::invocable<T, ReadBuffer&> && 
					 std::same_as<std::invoke_result_t<T, ReadBuffer&>, std::string_view>;

	class LineParseBuffer {
	private:
		ReadBuffer m_readBuff;
		std::string m_cumulativeBuff;
		size_t m_offset = 0;
	public:
		LineParseBuffer() {
			std::ranges::fill(m_readBuff, '\0');
		}
	private:
		template<Reader Reader, typename Substr, std::invocable GetSearchableText>
		size_t readUntil(Reader childReader, Substr substr, GetSearchableText getSearchableText) {
			while (true) {
				auto searchableText = getSearchableText();
				auto substrIndex = searchableText.find(substr);
				if (substrIndex == std::string_view::npos) {
					m_cumulativeBuff.append(childReader(m_readBuff));
				} else {
					return substrIndex;
				}
			}
			std::unreachable();
		}
	public:
		template<Reader Reader>
		std::string parse(std::string_view lineBegin, Reader childReader) {
			auto relativeLineBeginIndex = readUntil(childReader, lineBegin, [this] {
				return std::string_view{ m_cumulativeBuff.data() + m_offset, m_cumulativeBuff.size() - m_offset };
			});
			auto absLineBeginIndex = m_offset + relativeLineBeginIndex;
			auto absAfterLineBeginIndex = absLineBeginIndex + lineBegin.size();
			auto relativeNewlineIndex = readUntil(childReader, '\n', [&, this] {
				return std::string_view{ m_cumulativeBuff.data() + absAfterLineBeginIndex };
			});
			auto absNewlineIndex = absAfterLineBeginIndex + relativeNewlineIndex;

			m_offset = absNewlineIndex;

			return m_cumulativeBuff.substr(absLineBeginIndex, absNewlineIndex - absLineBeginIndex);
		}

		void clear() {
			m_cumulativeBuff.clear();
			m_offset = 0;
		}
	};

#ifdef _WIN64
	class WindowsPipe : public OSPipe {
	private:
		PROCESS_INFORMATION m_pi;
		HANDLE m_stdoutR = nullptr, m_stdoutW = nullptr;
		HANDLE m_stdinR  = nullptr, m_stdinW  = nullptr;
		LineParseBuffer m_lineParseBuff;

		static void destroyHandle(HANDLE& h) {
			if (h) {
				CloseHandle(h);
				h = nullptr;
			}
		}

		struct ParentProcessData {
			std::string exePath;
			std::string workingDirectory;
		};
		static ParentProcessData getProcessData() {
			std::array<char, MAX_PATH> buff;
			GetModuleFileNameA(nullptr, buff.data(), MAX_PATH);

			std::string enginePath{ buff.data(), std::strlen(buff.data()) };

			auto slashToEnd = std::ranges::find_last_if(enginePath, [](char c)-> bool {
				return c == '\\' || c == '/';
			});
			std::string engineDir;
			engineDir.append_range(std::ranges::subrange(enginePath.begin(), slashToEnd.begin()));

			return { enginePath, engineDir };
		}
	public:
		WindowsPipe() {
			ZeroMemory(&m_pi, sizeof(m_pi));
			
			SECURITY_ATTRIBUTES sa = { sizeof(sa), nullptr, true };
			auto createPipeImpl = [&sa](HANDLE& readHandle, HANDLE& writeHandle, HANDLE& uninheritedHandle) {
				if (!CreatePipe(&readHandle, &writeHandle, &sa, 0)) {
					std::println("Error: could not create pipe: {}", GetLastError());
					std::exit(-1);
				}
				SetHandleInformation(uninheritedHandle, HANDLE_FLAG_INHERIT, false);
			};
			createPipeImpl(m_stdinR, m_stdinW, m_stdinW);
			createPipeImpl(m_stdoutR, m_stdoutW, m_stdoutR);

			STARTUPINFOA si = { 0 };
			si.cb = sizeof(STARTUPINFOA);
			si.dwFlags = STARTF_USESTDHANDLES;
			si.hStdInput = m_stdinR;
			si.hStdOutput = m_stdoutW;
			si.hStdError = m_stdoutW;

			auto [path, dir] = getProcessData();

			auto res = CreateProcessA(path.c_str(), nullptr, nullptr, nullptr, true, CREATE_NEW_CONSOLE, nullptr,
				dir.c_str(), &si, &m_pi);
			if (!res) {
				std::println("Error: could not start separate process: {}", GetLastError());
				std::exit(-1);
			}

			//destroy handles not used by the parent
			destroyHandle(m_stdinR);
			destroyHandle(m_stdoutW);
		}

		void write(std::string_view command) override {
			DWORD totalBytesWritten = 0;
			DWORD bytesWritten = 0;
			while (std::cmp_less(totalBytesWritten, command.size())) {
				auto remainingCommand = command.data() + totalBytesWritten;
				auto remainingBytes = static_cast<DWORD>(command.size()) - totalBytesWritten;
				if (!WriteFile(m_stdinW, remainingCommand, remainingBytes, &bytesWritten, nullptr)) {
					std::println("Failed to write {}: {}", command, GetLastError());
					std::exit(-1);
				}
				totalBytesWritten += bytesWritten;
			}
		}
	private:
		std::string_view readFromChild(ReadBuffer& readBuff) {
			DWORD dwRead = 0;
			auto readRes = ReadFile(m_stdoutR, readBuff.data(), sizeof(readBuff) - 1, &dwRead, nullptr);
			if (!readRes) {
				std::println("Error: child process stopped writing data: {}", GetLastError());
				std::exit(-1);
			}
			readBuff[dwRead] = '\0';
			return std::string_view{ readBuff.data(), std::strlen(readBuff.data()) };
		}
	public:
		std::string read(std::string_view lineBegin) override {
			return m_lineParseBuff.parse(lineBegin, [this](auto& readBuff) {
				return readFromChild(readBuff);
			});
		}

		~WindowsPipe() override {
			destroyHandle(m_stdoutR);
			destroyHandle(m_stdoutW);
			destroyHandle(m_stdinR);
			destroyHandle(m_stdinW);
			if (m_pi.hProcess) { //kill child process
				if (!TerminateProcess(m_pi.hProcess, 1)) { 
					std::println("Error: failed to destroy child process: {}", GetLastError());
					WaitForSingleObject(m_pi.hProcess, INFINITE);
				}
			}
			destroyHandle(m_pi.hProcess);
			destroyHandle(m_pi.hThread);
		}
	};
	using PipeInheritor = WindowsPipe;
#else
	static_assert("No pipe implemented for non-windows OS yet!")
#endif

	Pipe::Pipe() : m_osPipe{ std::make_unique<PipeInheritor>() }
	{
	}

	void Pipe::write(std::string_view command) const {
		if (command.back() != '\n') {
			std::println("Error: {} command does not end with newline", command);
			std::exit(-1);
		}
		m_osPipe->write(command);
	}

	std::string Pipe::read(std::string_view expectedBegin) const {
		return m_osPipe->read(expectedBegin);
	}
}