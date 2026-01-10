module Chess.MoveGeneration:LegalMoveGeneration;

import nlohmann.json;

import std;

import Chess.Assert;
import Chess.BitboardImage;
import Chess.PieceMap;
import Chess.Profiler;
import Chess.RankCalculator;

import :ChainedMoveGenerator;
import :KingMoveGeneration;
import :KnightMoveGeneration;
import :PawnMoveGeneration;
import :PieceAttackers;
import :PieceLocations;
import :ReverseAttackGenerator;
import :SlidingMoveGenerators;

namespace chess {
	template<typename T>
	concept MoveAdder = std::invocable<T, MoveVector&, Move>;

	template<bool White, typename AllyPawnMoveGenerator, typename AllyPawnAttackGenerator,
			 typename EnemyPawnMoveGenerator, typename EnemyPawnAttackGenerator, Bitboard PromotionRank, 
			 bool DrawingBitboards>
	struct MoveGeneratorImpl {
		static inline AllyPawnMoveGenerator allyPawnMoveGenerator;
		static inline AllyPawnAttackGenerator allyPawnAttackGenerator;
		static inline EnemyPawnMoveGenerator enemyPawnMoveGenerator;
		static inline EnemyPawnAttackGenerator enemyPawnAttackGenerator;

		static_assert(PawnMoveGenerator<AllyPawnMoveGenerator>);
		static_assert(PawnMoveGenerator<EnemyPawnMoveGenerator>);

		static constexpr auto PAWN_ADDER = [](MoveVector& moves, Move move) {
			if (makeBitboard(move.to) & PromotionRank) {
				constexpr std::array PROMOTION_PIECES{ Queen, Rook, Bishop, Knight };
				for (auto piece : PROMOTION_PIECES) {
					move.promotionPiece = piece;
					moves.push_back(move);
				}
			} else {
				moves.push_back(move);
			}
		};

		static constexpr auto DEFAULT_MOVE_ADDER = [](MoveVector& moves, const Move& move) {
			moves.push_back(move);
		};

		static void drawPieceLocations(const PieceLocationData& pieceLocations, const std::string& filename) {
			auto colorGetter = [&](Bitboard bit) -> RGB {
				if (pieceLocations.allies & bit) {
					return LIGHT_TAN;
				} else if (pieceLocations.enemies & bit) {
					return BROWN;
				} else {
					return WHITE;
				}
			};
			drawBitboardImage(colorGetter, filename);
		}

		static void calcEnemyDestSquares(PieceMap<MoveGen>& enemyDestSquares, const PieceState& enemies, const PieceLocationData& pieceLocations) {
			enemyDestSquares[King] = kingMoveGenerator(enemies[King], pieceLocations.empty);
			enemyDestSquares[Bishop] = bishopMoveGenerator(enemies[Bishop], pieceLocations.empty);
			enemyDestSquares[Rook]  = rookMoveGenerator(enemies[Rook], pieceLocations.empty);
			enemyDestSquares[Queen] = queenMoveGenerator(enemies[Queen], pieceLocations.empty);
			enemyDestSquares[Knight] = knightMoveGenerator(enemies[Knight], pieceLocations.empty);
			enemyDestSquares[Pawn] = enemyPawnAttackGenerator(enemies[Pawn], ALL_SQUARES); //ensure pawns actually defend fellow enemy pieces
		}

		struct PinData {
			bool mustMoveInPinRay = false;
			Bitboard pinRay = 0; //does not include the pinner square
			Square pinnerSquare = Square::None;
			Piece pinnerPieceType = Piece::None;
		};
		using PinMap = SquareMap<PinData>;

		static PinData calcPinData(Square allySquare, const PieceState& enemies, const AttackerData& kingAttackerData, 
			const PieceLocationData& pieceLocations)
		{
			PinData ret;

			auto newEmpty = pieceLocations.empty | makeBitboard(allySquare);
			auto newSlidingAttackers = calcSlidingAttackers(enemies, newEmpty, pieceLocations.allyKing);
			auto newAttacker = newSlidingAttackers.attackers.calcAllLocations() & ~kingAttackerData.attackers.calcAllLocations();
			
			if (!newAttacker) { //no new king attackers, piece is not pinned
				return ret;
			}
			auto newRay = newSlidingAttackers.allRays() & ~kingAttackerData.allRays();

			ret.mustMoveInPinRay = true;
			ret.pinRay = newRay; 
			ret.pinnerSquare = nextSquare(newAttacker);
			ret.pinnerPieceType = enemies.findPiece(ret.pinnerSquare);

			return ret;
		}

		static PinMap calcPinnedAllies(const PieceState& allies, const PieceState& enemies, const Bitboard attackedAllies, 
			const AttackerData& kingAttackerData, const PieceLocationData& pieceLocations)
		{
			ProfilerLock l{ getCalcPinnedAlliesProfiler() };

			PinMap ret;

			auto calcPinnedAlliesImpl = [&](Bitboard pieces) {
				auto attackedPieces = pieces & attackedAllies;
				auto currSquare = Square::None;
				while (nextSquare(attackedPieces, currSquare)) {
					ret[currSquare] = calcPinData(currSquare, enemies, kingAttackerData, pieceLocations);
				}
			};
			calcPinnedAlliesImpl(allies[Queen]);
			calcPinnedAlliesImpl(allies[Rook]);
			calcPinnedAlliesImpl(allies[Bishop]);
			calcPinnedAlliesImpl(allies[Knight]);
			calcPinnedAlliesImpl(allies[Pawn]);

			return ret;
		}

		template<MoveAdder MoveAdder>
		static void addMoves(PositionData& posData, Square piecePos, MoveGen destSquares, Piece pieceType,
			const PieceLocationData& pieceLocations, const PieceState& enemies, MoveAdder moveAdder)
		{
			ProfilerLock l{ getMoveAdderProfiler() };

			posData.getAllySquares()[pieceType] |= destSquares;

			Move move{ piecePos, Square::None, pieceType, Piece::None };

			while (nextSquare(destSquares.emptyDestSquares, move.to)) {
				moveAdder(posData.legalMoves, move);
			}
			auto capturedPieceSquares = destSquares.nonEmptyDestSquares & pieceLocations.enemies;
			while (nextSquare(capturedPieceSquares, move.to)) {
				move.capturedPiece = enemies.findPiece(move.to);
				moveAdder(posData.legalMoves, move);
			}
		}

		template<typename MoveGenerator>
		static Bitboard addMoves(PositionData& posData, const PinMap& pinMap, Bitboard pieces, Piece pieceType, 
			const PieceLocationData& pieceLocations, const PieceState& enemies, MoveGenerator moveGen, AttackerData kingAttackerData)
		{
			Bitboard allSquares = 0;

			auto currPiecePos = Square::None;
			while (nextSquare(pieces, currPiecePos)) {
				auto destSquares = [&] {
					if constexpr (PawnMoveGenerator<MoveGenerator>) {
						return moveGen(makeBitboard(currPiecePos), pieceLocations.empty, pieceLocations.enemies);
					} else {
						return moveGen(makeBitboard(currPiecePos), pieceLocations.empty);
					}
				}();
				if (pinMap[currPiecePos].mustMoveInPinRay) {
					destSquares.emptyDestSquares    &= pinMap[currPiecePos].pinRay;
					destSquares.nonEmptyDestSquares &= makeBitboard(pinMap[currPiecePos].pinnerSquare);
				}
				allSquares |= destSquares.all();

				auto attackers = kingAttackerData.attackers.calcAllLocations();
				if (attackers) {
					destSquares &= (kingAttackerData.rays | attackers);
				}
				if constexpr (PawnMoveGenerator<MoveGenerator>) {
					addMoves(posData, currPiecePos, destSquares, pieceType, pieceLocations, enemies, PAWN_ADDER);
				} else {
					addMoves(posData, currPiecePos, destSquares, pieceType, pieceLocations, enemies, DEFAULT_MOVE_ADDER);
				}
			}
			return allSquares;
		}

		static void addKingMoves(PositionData& posData, Bitboard allEnemySquares, const Position::ImmutableTurnData& turnData, 
			const PieceLocationData& pieceLocations, const AttackerData& attackerData)
		{
			auto kingMoves = kingMoveGenerator(pieceLocations.allyKing, pieceLocations.empty);
			kingMoves &= ~allEnemySquares;

			if constexpr (DrawingBitboards) {
				drawBitboardImage(getDefaultColorGetter(kingMoves.all()), "king_moves.png");
				drawBitboardImage(getDefaultColorGetter(pieceLocations.allyKing, { 0, 255, 0 }), "ally_king.png");
			}

			posData.isCheck = (allEnemySquares & pieceLocations.allyKing);
			auto isCastlingClear = [&](const auto& castleMove) {
				auto squaresClear = (castleMove.squaresBetweenRookAndKing & ~pieceLocations.empty) == 0;
				auto squaresNotChecked = (castleMove.mandatoryUncheckedSquares & allEnemySquares) == 0;
				return (!posData.isCheck && squaresClear && squaresNotChecked);
			};
			if (!posData.isCheck) {
				if (turnData.allies.castling.canCastleKingside() && isCastlingClear(turnData.allyKingside)) {
					kingMoves.emptyDestSquares |= makeBitboard(turnData.allyKingside.kingTo);
				}
				if (turnData.allies.castling.canCastleQueenside() && isCastlingClear(turnData.allyQueenside)) {
					kingMoves.emptyDestSquares |= makeBitboard(turnData.allyQueenside.kingTo);
				}
			} else {
				kingMoves &= ~attackerData.indirectRays;
			}
			addMoves(posData, nextSquare(pieceLocations.allyKing), kingMoves, King, pieceLocations, turnData.enemies, DEFAULT_MOVE_ADDER);
		}

		static void addEnPassantMoves(PositionData& posData, const PinMap& pinMap, const AttackerData& kingAttackers, 
			const PieceState& enemies, Bitboard pawns, Square jumpedEnemyPawn, const PieceLocationData& pieceLocations)
		{
			ProfilerLock l{ getEnPessantProfiler() };

			auto enPassantData = allyPawnAttackGenerator(pawns, jumpedEnemyPawn);
			if (!enPassantData.pawns) {
				return;
			}
			auto from = Square::None;
			while (nextSquare(enPassantData.pawns, from)) {
				if (pinMap[from].mustMoveInPinRay) { //impossible for an en passant pawn to be pinned by a pawn, so it can't take the double jumped pawn
					continue;
				}
				PieceLocationData newPieceLocations{
					pieceLocations.allyKing,
					pieceLocations.allies & ~makeBitboard(from) | makeBitboard(enPassantData.squareInFrontOfEnemyPawn),
					pieceLocations.enemies & ~makeBitboard(jumpedEnemyPawn)
				};
				auto newSlidingAttackers = calcSlidingAttackers(enemies, newPieceLocations.empty, newPieceLocations.allyKing);
				if ((newSlidingAttackers.attackers.calcAllLocations() & ~kingAttackers.attackers.calcAllLocations()) != 0) { //ally pawn is actually pinned from behind the enemy pawn
					continue;
				}
				posData.legalMoves.emplace_back(from, enPassantData.squareInFrontOfEnemyPawn, jumpedEnemyPawn, Pawn, Pawn, Piece::None);
				MoveGen gen{ 0, makeBitboard(enPassantData.squareInFrontOfEnemyPawn) };
				posData.getAllySquares()[Pawn] |= gen;
			}
		}

		static PositionData calcAllLegalMoves(const Position::ImmutableTurnData& turnData) {
			PositionData ret{ White };
			auto& allies  = turnData.allies;
			auto& enemies = turnData.enemies;
			
			PieceLocationData pieceLocations{ allies[King], allies.calcAllLocations(), enemies.calcAllLocations() };
			
			if constexpr (DrawingBitboards) {
				drawPieceLocations(pieceLocations,"normal_piece_locations.png");
			}

			calcEnemyDestSquares(ret.getEnemySquares(), enemies, pieceLocations);
			auto allEnemySquares = ret.allEnemySquares();

			auto kingAttackerData = calcAttackers(White, enemies, pieceLocations.empty, pieceLocations.allyKing);
			addKingMoves(ret, allEnemySquares, turnData, pieceLocations, kingAttackerData);

			if (kingAttackerData.hasMultipleAttackers()) { //if there are multiple checks, we have to move the king
				return ret;
			}

			auto attackedAllies = allEnemySquares & pieceLocations.allies;

			auto pinMap = calcPinnedAllies(allies, enemies, attackedAllies, kingAttackerData, pieceLocations);
			
			auto addMovesImpl = [&](Piece piece, auto moveGenerator) {
				addMoves(ret, pinMap, allies[piece], piece, pieceLocations, enemies, moveGenerator, kingAttackerData);
			};

			addMovesImpl(Queen, queenMoveGenerator);
			addMovesImpl(Rook, rookMoveGenerator);
			addMovesImpl(Bishop, bishopMoveGenerator);
			addMovesImpl(Knight, knightMoveGenerator);
			addMovesImpl(Pawn, allyPawnMoveGenerator);

			if (enemies.doubleJumpedPawn != Square::None) {
				addEnPassantMoves(ret, pinMap, kingAttackerData, enemies, allies[Pawn], enemies.doubleJumpedPawn, pieceLocations);
			}

			return ret;
		}
	};

	template<bool DrawingBitboards>
	PositionData calcAllLegalMovesImpl(const Position& pos) {
		ProfilerLock l{ getLegalMoveGenerationProfiler() };

		auto turnData = pos.getTurnData();

		if (turnData.isWhite) {
			using MoveGenerator = MoveGeneratorImpl<true, WhitePawnMoveGenerator, WhitePawnAttackGenerator,
				BlackPawnMoveGenerator, BlackPawnAttackGenerator, calcRank<8>(), DrawingBitboards>;
			return MoveGenerator::calcAllLegalMoves(turnData);
		} else {
			using MoveGenerator = MoveGeneratorImpl<false, BlackPawnMoveGenerator, BlackPawnAttackGenerator,
				WhitePawnMoveGenerator, WhitePawnAttackGenerator, calcRank<1>(), DrawingBitboards>;
			return MoveGenerator::calcAllLegalMoves(turnData);
		}
	}

	PositionData calcPositionData(const Position& pos) {
		return calcAllLegalMovesImpl<false>(pos);
	}
	PositionData calcPositionDataAndDrawBitboards(const Position& pos) {
		return calcAllLegalMovesImpl<true>(pos);
	}
}