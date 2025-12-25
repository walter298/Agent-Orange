export module Chess.Time;

export import std;

export namespace chess {
	void setBaseline(int depth);
	void setTime(int depth, int pieceCount, std::chrono::nanoseconds timeCalculated);
	int getMaxDepth();
}