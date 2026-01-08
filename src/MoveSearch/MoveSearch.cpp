module Chess.MoveSearch;

import std;
import BS.thread_pool;

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
		bool invalidTTEntry = false;
	};

	class Searcher {
	private:
		static constexpr SafeUnsigned<std::uint8_t> RANDOMIZATION_CUTOFF{ 2 };
		std::mt19937 m_urbg;
		bool m_helper = false;
		const std::atomic_bool* m_stopRequested;
	public:
		Searcher(bool helper, std::atomic_bool* stopRequested)
			: m_urbg{ std::random_device{}() }, m_helper{ helper }, m_stopRequested{ stopRequested }
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

			if constexpr (std::same_as<Ret, PositionRating>) {
				if (node.getRepetitionMap().getPositionCount(node.getPos()) >= 3) {
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

			if constexpr (std::same_as<Ret, PositionRating>) { //node will not be done if we are at the root, so it is safe to have this check only for ratings
				if (m_stopRequested->load()) {
					return { node.getRating(), false };
				}
			}
			
			if (!m_stopRequested->load()) {
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
			}
			if constexpr (std::same_as<Ret, PositionRating>) { //node will not be done if we are at the root, so it is safe to have this check only for ratings
				if (node.isDone()) {
					return { node.getRating(), false }; //already checked whether position is a repetition
				} else if (m_stopRequested->load()) {
					return { node.getRating(), true };
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

			if (!bestRating.invalidTTEntry) {
				PositionEntry newEntry{ bestMove, bestRating.rating, node.getRemainingDepth(), bound };
				storePositionEntry(node.getPos(), newEntry);
			}

			bestRating.invalidTTEntry = false; //don't propagate repetition flag up the tree

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
		std::optional<Move> operator()(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
			if (pos.isWhite()) {
				debugPrint("Searching for white");
				return iterativeDeepening<true>(pos, depth, repetitionMap);
			} else {
				debugPrint("Searching for black");
				return iterativeDeepening<false>(pos, depth, repetitionMap);
			}
		}
	};

	const auto THREAD_COUNT = std::thread::hardware_concurrency();
	constexpr auto MAIN_THREAD_INDEX = 0uz;

	struct AsyncSearchState {
		BS::thread_pool<> pool{ THREAD_COUNT };
		std::atomic_bool stopRequested = false;
		std::vector<Searcher> searchers;

		AsyncSearchState() {
			searchers.reserve(THREAD_COUNT);
			searchers.emplace_back(false, &stopRequested); //insert main thread
			if (THREAD_COUNT > 1) {
				for (auto i = 0uz; i < THREAD_COUNT - 1; i++) { //insert helper threads
					searchers.emplace_back(true, &stopRequested);
				}
			}
		}
	};

	AsyncSearch::AsyncSearch() 
		: m_state{ std::make_shared<AsyncSearchState>() }
	{
	}

	Move voteForBestMove(const std::vector<std::optional<Move>>& moves) {
		std::unordered_map<Move, int, MoveHasher> moveOccurrences;
		auto bestMove = Move::null();
		auto bestMoveCount = 0;

		for (const auto& move : moves) {
			auto& occ = moveOccurrences[*move];
			occ++;
			if (occ > bestMoveCount) {
				bestMoveCount = occ;
				bestMove = *move;
			}
		}
		return bestMove;
	} 

	std::optional<Move> findBestMoveImpl(std::shared_ptr<AsyncSearchState> state, Position pos, SafeUnsigned<std::uint8_t> depth, RepetitionMap repetitionMap) {
		debugPrint(std::format("Searching depth {}", depth.get()));

		ProfilerLock l{ getBestMoveProfiler() };

		state->stopRequested.store(false);

		auto moveCandidateFutures = state->pool.submit_sequence(0uz, state->searchers.size(), [&](size_t i) {
			SafeUnsigned sIdx{ static_cast<std::uint8_t>(i) };
			auto d = ((sIdx % 2_su8 == 0_su8) ? 0_su8 : 1_su8);
			return state->searchers[i](pos, std::max(depth - d, 1_su8), repetitionMap);
		});
		
		auto moveCandidates = moveCandidateFutures.get();
		zAssert(!moveCandidates.empty());

		if (!moveCandidates.front()) { //if one move is empty, all moves are empty
			return std::nullopt;
		}

		return voteForBestMove(moveCandidates);
	}

	std::optional<Move> AsyncSearch::findBestMove(const Position& pos, SafeUnsigned<std::uint8_t> depth, const RepetitionMap& repetitionMap) {
		return findBestMoveImpl(m_state, pos, depth, repetitionMap);
	}

	void AsyncSearch::cancel() {
		m_state->stopRequested.store(true);
	}
}
