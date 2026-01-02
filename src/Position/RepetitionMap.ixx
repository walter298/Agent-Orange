module;

#include <boost/unordered/unordered_flat_map.hpp>

export module Chess.Position.RepetitionMap;

import Chess.Position;

export namespace chess {
	class RepetitionMap {
	private:
		boost::unordered_flat_map<std::uint64_t, int> m_positionCounts;
	public:
		void push(const Position& pos);
		void pop(const Position& pos);
		int getPositionCount(const Position& pos) const;
		void clear();
		int getTotalPositionCount() const;
	};
}