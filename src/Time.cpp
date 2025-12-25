module Chess.Time;

import std;

import Chess.Assert;

namespace chess {
	using namespace std::literals;

	class TimeMap {
	private:
		static constexpr auto MAX_DEPTH = 20;
		static constexpr auto MAX_PIECE_COUNT = 32;

		struct TimeData {
			std::chrono::nanoseconds sum{ 0 };
			int timesRan = 0;

			std::chrono::nanoseconds getAverage() const {
				return sum / timesRan;
			}
		};
		using PieceTimeData = std::array<TimeData, MAX_PIECE_COUNT>;
		std::array<PieceTimeData, MAX_DEPTH> m_data;
	public:
		TimeMap() {
			PieceTimeData defaultTimeData;
			std::ranges::fill(defaultTimeData, TimeData{});
			std::ranges::fill(m_data, defaultTimeData);
		}

		std::chrono::nanoseconds getAverage(int depth, int pieceCount) const {
			zAssert(depth > 0 && depth <= MAX_DEPTH);
			zAssert(pieceCount >= 2 && pieceCount <= MAX_PIECE_COUNT);
			return m_data[static_cast<size_t>(depth)][static_cast<size_t>(pieceCount)].getAverage();
		}
	};
	TimeMap timeMap;
	int baseDepth = 0;

	void setBaseline(int depth) {
		baseDepth = depth;
	}
	void setTime(int depth, int pieceCount, std::chrono::nanoseconds timeCalculated) {
		
	}
	int getMaxDepth() {
		
	}
}