export module Chess.EnvironmentVariable;

import std;

export namespace chess {
	std::filesystem::path getProfilingSessionsDirectoryPath();
	std::filesystem::path getBitboardImageDirectoryPath();
}