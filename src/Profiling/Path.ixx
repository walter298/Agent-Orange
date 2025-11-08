export module Chess.Profiler:Path;

export import std;

export namespace chess {
	std::filesystem::path getProfilingSessionsDirectoryPath();
	std::filesystem::path getProfilingSessionFilePath();
}