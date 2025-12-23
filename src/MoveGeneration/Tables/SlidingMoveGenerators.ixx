export module Chess.MoveGeneration:SlidingMoveGenerators;

import :ChainedMoveGenerator;
import :MoveGen;
import :TableLoader;

namespace chess {
	_forceinline const MagicMaps<StaticBMI>& getMaps() {
		static const auto maps = loadMaps();
		return maps;
	}

	_forceinline MoveGen getSlidingMovesImpl(const SquareMap<StaticBMI>& map, Bitboard movingPieces, Bitboard empty) {
		Bitboard ret = 0;

		auto currSquare = None;
		while (nextSquare(movingPieces, currSquare)) {
			auto blockers = ~empty & ~currSquare;
			auto allSquares = map[currSquare][blockers];
			ret |= allSquares;
		}
		return { ret & empty, ret & ~empty };
	}

	struct BishopAttackGenerator {
		_forceinline MoveGen operator()(Bitboard movingPieces, Bitboard empty) const {
			return getSlidingMovesImpl(getMaps().diagonalMoveMap, movingPieces, empty);
		}
	};
	export constexpr BishopAttackGenerator bishopMoveGenerator;

	struct RookAttackGenerator {
		_forceinline MoveGen operator()(Bitboard movingPieces, Bitboard empty) const {
			return getSlidingMovesImpl(getMaps().orthogonalMoveMap, movingPieces, empty);
		}
	};
	export constexpr RookAttackGenerator rookMoveGenerator;

	export constexpr ChainedMoveGenerator queenMoveGenerator{ bishopMoveGenerator, rookMoveGenerator };

	template<typename MoveGenerator>
	concept SlidingMoveGenerator = std::same_as<BishopAttackGenerator, MoveGenerator> ||
								   std::same_as<RookAttackGenerator, MoveGenerator> ||
							       std::same_as<std::remove_cvref_t<decltype(queenMoveGenerator)>, MoveGenerator>;
}