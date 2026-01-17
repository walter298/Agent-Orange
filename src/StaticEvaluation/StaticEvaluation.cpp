module Chess.Evaluation;

import std;

import Chess.Profiler;
import Chess.Position.PieceState;

import :Constants;
import :Material;
import :PawnStructure;
import :PieceDevelopment;
import :KingSafety;

namespace chess {
	Rating calcAttackRating(const Position& pos, const PositionData& posData) {
		auto [white, black] = pos.getColorSides();
		
		auto getAttackedPiecesRating = [&](const PieceState& pieceState, Bitboard enemySquares) -> Rating {
			auto attackedPieces = pieceState.calcAllLocations() & enemySquares;
			auto attackedPieceCount = std::popcount(attackedPieces);
			return static_cast<Rating>(attackedPieceCount) * ATTACKED_PIECE_RATING;
		};
		auto allWhiteSquares = posData.whiteSquares.destSquaresPinConsidered;
		auto allBlackSquares = posData.blackSquares.destSquaresPinConsidered;
		return getAttackedPiecesRating(white, allBlackSquares) - getAttackedPiecesRating(black, allWhiteSquares);
	}

	Rating calcCastleRating(const Position& pos) {
		auto [white, black] = pos.getColorSides();

		auto getCastleRating = [](const auto& pieceState) {
			return pieceState.castling.hasCastledKingside() || pieceState.castling.hasCastledQueenside() ? CASTLE_RATING : 0_rt;
		};
		return getCastleRating(white) - getCastleRating(black);
	}

	Rating staticEvaluation(const Position& pos, const PositionData& posData) {
		ProfilerLock l{ getStaticEvaluationProfiler() };
		return calcCastleRating(pos) + calcMaterialRating(pos) + calcPawnStructureRating(pos) + calcAttackRating(pos, posData) + 
			   calcKingSafetyRating(pos, posData) + calcPieceDevelopmentRating(pos, posData);
	}

	Rating getPieceRating(Piece piece) {
		return pieceRatings[piece];
	}
}
