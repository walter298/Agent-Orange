module Chess.Evaluation;

import std;

import Chess.Profiler;

import :Constants;
import :Material;
import :PawnStructure;
import :KingSafety;

namespace chess {
	Rating calcAttackRating(const Position& pos, const PositionData& posData) {
		auto [white, black] = pos.getColorSides();
		
		auto getAttackedPiecesRating = [&](const PieceState& pieceState, Bitboard enemySquares) -> Rating {
			auto attackedPieces = pieceState.calcAllLocations() & enemySquares;
			auto attackedPieceCount = std::popcount(attackedPieces);
			return static_cast<Rating>(attackedPieceCount) * ATTACKED_PIECE_RATING;
		};
		return getAttackedPiecesRating(white, posData.blackSquares) - getAttackedPiecesRating(black, posData.whiteSquares);
	}

	Rating staticEvaluation(const Position& pos, const PositionData& posData) {
		ProfilerLock l{ getStaticEvaluationProfiler() };
		return calcMaterialRating(pos) + calcPawnStructureRating(pos) + calcAttackRating(pos, posData) + calcKingSafetyRating(pos, posData);
	}

	Rating getPieceRating(Piece piece) {
		return pieceRatings[piece];
	}
}
