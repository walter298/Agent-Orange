module;

#include <unifex/when_all_range.hpp>
#include <unifex/static_thread_pool.hpp>
#include <unifex/sync_wait.hpp>
#include <unifex/task.hpp>

module Chess.MoveSearch;

import std;

import Chess.Assert;
import Chess.DebugPrint;
import Chess.EasyRandom;
import Chess.Evaluation;
import Chess.MoveGeneration;
import Chess.Position.RepetitionMap;
import Chess.Profiler;
import Chess.Rating;

import :KillerMoveHistory;
import :MoveOrdering;
import :MoveHasher;
import :Node;
import :PositionTable;

namespace chess {
	class AlphaBeta {
	private:
		Rating m_alpha = worstPossibleRating<true>();
		Rating m_beta = worstPossibleRating<false>();
	public:
		void updateAlpha(Rating childRating) {
			m_alpha = std::max(childRating, m_alpha);
		}
		void updateBeta(Rating childRating) {
			m_beta = std::min(childRating, m_beta);
		}

		template<bool Maximizing>
		void update(Rating childRating) {
			if constexpr (Maximizing) {
				updateAlpha(childRating);
			}
			else {
				updateBeta(childRating);
			}
		}

		bool canPrune() const {
			return m_beta <= m_alpha;
		}

		Rating getAlpha() const {
			return m_alpha;
		}
		Rating getBeta() const {
			return m_beta;
		}
	};

	struct PositionRating {
		Rating rating = 0_rt;
		bool isRepetition = false;
	};

	using MoveTask  = unifex::task<std::optional<Move>>;
	using Scheduler = unifex::static_thread_pool::scheduler;

	class Searcher {
	private:
		static constexpr SafeUnsigned<std::uint8_t> RANDOMIZATION_CUTOFF{ 2 };
		std::mt19937 m_urbg;
		bool m_helper = false;
		Scheduler m_scheduler;
	public:
		Searcher(bool helper, Scheduler scheduler)
			: m_urbg{ std::random_device{}() }, m_helper{ helper }, m_scheduler{ scheduler }
		{
		}
	private:
		static bool wouldMakeRepetition(const Position& pos, Move pvMove, const RepetitionMap& repetitionMap) {
			Position child{ pos, pvMove };
			auto repetitionCount = repetitionMap.getPositionCount(child) + 1; //add 1 since we haven't actually pushed this position yet
			return repetitionCount >= 2; //return 2 (not 3) because the opposing player could then make a threefold repetition after this
		}

		template<typename Ret, bool Maximizing>
		Ret minimax(const Node& node, AlphaBeta alphaBeta) {
			if (node.getPositionData().legalMoves.empty()) {
				if constexpr (std::same_as<Ret, PositionRating>) {
					auto rating = node.getPositionData().isCheckmate ? checkmatedRating<Maximizing>() : 0_rt;
					return PositionRating{ rating, false };
				} else {
					return std::nullopt;
				}
			}

			if (node.getRepetitionMap().getPositionCount(node.getPos()) >= 3) {
				if constexpr (std::same_as<Ret, PositionRating>) {
					return { 0_rt, true };
				}
			}

			auto pvMove = Move::null();

			auto returnImpl = [&](const PositionEntry& entry) {
				if constexpr (std::same_as<Ret, PositionRating>) {
					return PositionRating{ entry.rating, false };
				} else { //don't prune at the root if we are a helper
					return std::optional{ entry.bestMove };
				} 
			};
			
			if (auto entryRes = getPositionEntry(node.getPos())) {
				const auto& entry = *entryRes;
				pvMove = entry.bestMove;
				bool canUseEntry = !(m_helper && node.getLevel() == 0_su8);
				if (!wouldMakeRepetition(node.getPos(), entry.bestMove, node.getRepetitionMap()) && entry.depth >= node.getRemainingDepth()) {
					switch (entry.bound) {
					case InWindow:
						if (canUseEntry) {
							return returnImpl(entry);
						}
						break;
					case LowerBound:
						if (entry.rating >= alphaBeta.getBeta()) {
							if (canUseEntry) {
								return returnImpl(entry);
							}
						} else {
							alphaBeta.updateAlpha(entry.rating);
						}
						break;
					case UpperBound:
						if (entry.rating <= alphaBeta.getAlpha()) {
							if (canUseEntry) {
								return returnImpl(entry);
							}
						} else {
							alphaBeta.updateBeta(entry.rating);
						}
						break;
					}
				}
			}
			if constexpr (std::same_as<Ret, PositionRating>) { //node will not be done if we are at the root, so it is safe to have this check only for ratings
				if (node.isDone()) {
					return { node.getRating(), false }; //already checked whether position is a repetition
				}
			}
			return bestChildPosition<Ret, Maximizing>(node, pvMove, alphaBeta);
		}

		template<typename Ret, bool Maximizing>
		Ret bestChildPosition(const Node& node, const Move& pvMove, AlphaBeta alphaBeta) {
			auto originalAlphaBeta = alphaBeta;
			auto movePriorities = [&] {
				constexpr SafeUnsigned<std::uint8_t> RANDOMIZATION_CUTOFF{ 3 };
				if (m_helper && node.getLevel() < RANDOMIZATION_CUTOFF) {
					std::vector ret{ std::from_range, node.getPositionData().legalMoves | std::views::transform([&](const auto& move) {
						return MovePriority{ move, node.getRemainingDepth() - 1_su8 };
					}) };
					std::ranges::shuffle(ret, m_urbg);
					return FixedVector{ std::move(ret) };
				} else {
					return getMovePriorities(node, pvMove);
				}
			}();
			auto bestMove = Move::null();
			PositionRating bestRating{ worstPossibleRating<Maximizing>(), false };

			auto bound = InWindow;
			bool didNotPrune = true;

			for (const auto& movePriority : movePriorities) {
				auto child = Node::makeChild(node, movePriority);
				auto childRating = minimax<PositionRating, !Maximizing>(child, alphaBeta);

				if constexpr (Maximizing) {
					if (childRating.rating > bestRating.rating) {
						bestRating = childRating;
						bestMove = movePriority.getMove();
					}
				} else {
					if (childRating.rating < bestRating.rating) {
						bestRating = childRating;
						bestMove = movePriority.getMove();
					}
				}

				alphaBeta.update<Maximizing>(bestRating.rating);
				if (alphaBeta.canPrune()) {
					updateHistoryScore(movePriority.getMove(), node.getRemainingDepth());
					bound = Maximizing ? LowerBound : UpperBound;
					didNotPrune = false;
					break;
				}
			}

			zAssert(bestMove != Move::null());
			if (didNotPrune) {
				if constexpr (Maximizing) {
					if (bestRating.rating <= originalAlphaBeta.getAlpha()) {
						bound = UpperBound;
					}
				} else {
					if (bestRating.rating >= originalAlphaBeta.getBeta()) {
						bound = LowerBound;
					}
				}
			}

			if (!bestRating.isRepetition) {
				PositionEntry newEntry{ bestMove, bestRating.rating, node.getRemainingDepth(), bound };
				storePositionEntry(node.getPos(), newEntry);
			}

			bestRating.isRepetition = false; //don't propagate repetition flag up the tree

			if constexpr (std::same_as<Ret, std::optional<Move>>) {
				return bestMove;
			} else {
				return bestRating;
			}
		}

		template<bool Maximizing>
		std::optional<Move> startAlphaBetaSearch(const Position& pos, SafeUnsigned<std::uint8_t> depth, RepetitionMap repetitionMap) {
			AlphaBeta alphaBeta;
			auto root = Node::makeRoot(pos, depth, repetitionMap);
			return minimax<std::optional<Move>, Maximizing>(root, alphaBeta);
		}

		template<bool Maximizing>
		std::optional<Move> iterativeDeepening(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
			for (auto iterDepth = 1_su8; iterDepth < depth; ++iterDepth) {
				startAlphaBetaSearch<Maximizing>(pos, iterDepth, repetitionMap);
			}
			return startAlphaBetaSearch<Maximizing>(pos, depth, repetitionMap);
		}
	public:
		MoveTask operator()(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
			co_await unifex::schedule(m_scheduler);

			if (pos.isWhite()) {
				co_return iterativeDeepening<true>(pos, depth, repetitionMap);
			} else {
				co_return iterativeDeepening<false>(pos, depth, repetitionMap);
			}
		}
	};

	constexpr auto THREAD_COUNT = 4;
	constexpr auto MAIN_THREAD_INDEX = 0uz;

	std::vector<Searcher> makeSearchers(unifex::static_thread_pool& pool) {
		std::vector<Searcher> searchers;
		searchers.reserve(THREAD_COUNT); 

		searchers.emplace_back(false, pool.get_scheduler()); //insert main thread
		for (int i = 0; i < THREAD_COUNT - 1; i++) {
			searchers.emplace_back(true, pool.get_scheduler());
		}
		return searchers;
	}

	struct VoteResult {
		Move bestMove = Move::null();
		int occ = 0;
	};
	VoteResult voteForBestMove(const std::vector<std::optional<Move>>& candidates) {
		int maxOcurrenceCount = 0;
		auto bestMove = Move::null();
		std::unordered_map<Move, int, MoveHasher> map;
		for (const auto& move : candidates) {
			zAssert(move.has_value());
			auto& moveCount = map[*move];
			moveCount++;
			if (moveCount > maxOcurrenceCount) {
				maxOcurrenceCount = moveCount;
				bestMove = *move;
			}
		}
		zAssert(bestMove != Move::null());
		return { bestMove, maxOcurrenceCount };
	}

	MoveTask findBestMoveImpl(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
		unifex::static_thread_pool pool{ THREAD_COUNT };
		auto searchers = makeSearchers(pool);

		std::vector<MoveTask> searcherTasks;
		searcherTasks.reserve(THREAD_COUNT);
		for (auto&& [idx, searcher] : std::views::enumerate(searchers)) {
			SafeUnsigned sIdx{ static_cast<std::uint8_t>(idx) };
			auto d = ((sIdx % 2_su8 == 0_su8) ? 1_su8 : 0_su8);
			searcherTasks.push_back(searcher(pos, std::max(depth - d, 1_su8), repetitionMap));
		}
		auto moveCandidates = co_await unifex::when_all_range(std::move(searcherTasks));

		if (!moveCandidates.front()) { //if one move is empty, all moves are empty
			co_return std::nullopt;
		}

		//threads vote for the most popular move
		auto [bestMove, occ] = voteForBestMove(moveCandidates);

		//tiebreak to main thread
		if (occ == 1) {
			co_return *moveCandidates[MAIN_THREAD_INDEX];
		}
		if (bestMove != *moveCandidates[MAIN_THREAD_INDEX]) {
			std::println("Main thread was overruled by vote of {}", occ);
		}
		for (const auto& move : moveCandidates | std::views::drop(1)) {
			if (move != *moveCandidates[MAIN_THREAD_INDEX]) {
				std::println("Another thread picked a different move from the main thread: {}", move->getUCIString());
			}
		}
		co_return bestMove;
	}

	std::optional<Move> findBestMove(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
		return unifex::sync_wait(findBestMoveImpl(pos, depth, repetitionMap)).value();
	}
}