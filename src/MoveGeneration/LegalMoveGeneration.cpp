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

//		static void drawEnemyLayoutBitboard(const PieceLocationData& pieceLocations,
//			const EnemyMoveData& enemyMoveData)
//		{
//			auto colorGetter = [&](Bitboard bit) -> RGB {
////				if (bit & enemyMoveData.checklines.kingAttackers) {
////					return PINK;
////				}
//				if (pieceLocations.allies & bit) {
//					if (bit & enemyMoveData.squares) { 
//						return RED; //ally under attack
//					} else {
//						return GREEN; //ally not under attack
//					}
//				}
//				if (pieceLocations.enemies & bit) {
//					if (bit & enemyMoveData.squares) {
//						return PURPLE; //defended enemy piece
//					}
//					return BROWN; //undefended enemy piece
//				}
//				if (enemyMoveData.squares & bit) {
//					return YELLOW; //empty square under attack
//				}
//				return WHITE; //empty square not under attack
//			};
//			drawBitboardImage(colorGetter, "enemy_layout_bitboard.png");
//		}

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

		static Bitboard calcEnemyDestSquares(const PieceState& enemies, const PieceLocationData& pieceLocations) {
			auto kingAttacks	 = kingMoveGenerator(enemies[King], pieceLocations.empty);
			auto diagonalAttacks = bishopMoveGenerator(enemies[Bishop] | enemies[Queen], pieceLocations.empty);
			auto rookAttacks     = rookMoveGenerator(enemies[Rook] | enemies[Queen], pieceLocations.empty);
			auto knightAttacks   = knightMoveGenerator(enemies[Knight], pieceLocations.empty);
			auto pawnAttacks     = enemyPawnAttackGenerator(enemies[Pawn], ALL_SQUARES); //ensure pawns actually defend fellow enemy pieces
			return (kingAttacks | diagonalAttacks | rookAttacks | knightAttacks | pawnAttacks).all();
		}

		struct PinData {
			bool cannotMove = false;
			bool mustCapturePinner = false;
			Square pinnerSquare = Square::None;
			Piece pinnerPieceType = Piece::None;
		};
		using PinMap = SquareMap<PinData>;

		template<typename MoveGenerator>
		static PinData calcPinData(Square allySquare, const PieceState& enemies, Bitboard slidingKingAttackers, 
			const PieceLocationData& pieceLocations, MoveGenerator moveGenerator)
		{
			PinData ret;

			auto newEmpty = pieceLocations.empty | makeBitboard(allySquare);
			auto newSlidingAttackers = calcSlidingAttackers(enemies, newEmpty, pieceLocations.allyKing);
			auto newAttacker = newSlidingAttackers.attackers.calcAllLocations() & ~slidingKingAttackers;

			if (!newAttacker) { //no new king attackers, piece is not pinned
				return ret;
			}

			//if there is a new attacker, see if we can capture it with the supposedly pinned piece (this is impossible for knights)
			if constexpr (SlidingMoveGenerator<MoveGenerator> || PawnMoveGenerator<MoveGenerator>) { 
				if (newAttacker) {
					auto from = makeBitboard(allySquare);
					auto reverseAttacks = [&] {
						if constexpr (SlidingMoveGenerator<MoveGenerator>) {
							return moveGenerator(from, newEmpty);
						} else {
							return moveGenerator(from, ALL_SQUARES);
						}
					}();
					if (reverseAttacks.nonEmptyDestSquares & newAttacker) {
						ret.mustCapturePinner = true;
						ret.pinnerSquare      = nextSquare(newAttacker);
						ret.pinnerPieceType   = enemies.findPiece(ret.pinnerSquare);
						return ret;
					}
				}
			}

			//we couldn't capture the new attacker
			ret.cannotMove = true;
			return ret;
		}

		static PinMap calcPinnedAllies(const PieceState& allies, const PieceState& enemies, const Bitboard attackedAllies, 
			const Bitboard slidingKingAttackers, const PieceLocationData& pieceLocations)
		{
			ProfilerLock l{ getCalcPinnedAlliesProfiler() };

			PinMap ret;

			auto calcPinnedAlliesImpl = [&](Bitboard pieces, auto moveGenerator) {
				auto attackedPieces = pieces & attackedAllies;
				auto currSquare = Square::None;
				while (nextSquare(attackedPieces, currSquare)) {
					ret[currSquare] = calcPinData(currSquare, enemies, slidingKingAttackers, pieceLocations, moveGenerator);
				}
			};
			calcPinnedAlliesImpl(allies[Queen], queenMoveGenerator);
			calcPinnedAlliesImpl(allies[Rook], rookMoveGenerator);
			calcPinnedAlliesImpl(allies[Bishop], bishopMoveGenerator);
			calcPinnedAlliesImpl(allies[Knight], knightMoveGenerator);
			calcPinnedAlliesImpl(allies[Pawn], allyPawnAttackGenerator);

			return ret;
		}

		template<MoveAdder MoveAdder>
		static void addMoves(MoveVector& moves, Square piecePos, MoveGen destSquares, Piece pieceType,
			const PieceLocationData& pieceLocations, const PieceState& enemies, MoveAdder moveAdder)
		{
			ProfilerLock l{ getMoveAdderProfiler() };
			
			Move move{ piecePos, Square::None, pieceType, Piece::None };

			while (nextSquare(destSquares.emptyDestSquares, move.to)) {
				moveAdder(moves, move);
			}
			auto capturedPieceSquares = destSquares.nonEmptyDestSquares & pieceLocations.enemies;
			while (nextSquare(capturedPieceSquares, move.to)) {
				move.capturedPiece = enemies.findPiece(move.to);
				moveAdder(moves, move);
			}
		}

		template<typename MoveGenerator>
		static Bitboard addMoves(MoveVector& moves, Bitboard movablePieces, Piece pieceType, const PieceLocationData& pieceLocations,
			const PieceState& enemies, MoveGenerator moveGen, AttackerData kingAttackerData)
		{
			Bitboard allSquares = 0;

			auto currPiecePos = Square::None;
			while (nextSquare(movablePieces, currPiecePos)) {
				auto destSquares = [&] {
					if constexpr (PawnMoveGenerator<MoveGenerator>) {
						return moveGen(makeBitboard(currPiecePos), pieceLocations.empty, pieceLocations.enemies);
					} else {
						return moveGen(makeBitboard(currPiecePos), pieceLocations.empty);
					}
				}();
				
				allSquares |= destSquares.all();

				auto attackers = kingAttackerData.attackers.calcAllLocations();
				if (attackers) {
					destSquares &= (kingAttackerData.rays | attackers);
				}
				if constexpr (PawnMoveGenerator<MoveGenerator>) {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, PAWN_ADDER);
				} else {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, DEFAULT_MOVE_ADDER);
				}
			}
			return allSquares;
		}

		static Bitboard addKingMoves(MoveVector& moves, const Position::ImmutableTurnData& turnData, 
			const PieceLocationData& pieceLocations, Bitboard enemyDestSquares, const AttackerData& attackerData)
		{
			auto kingMoves = kingMoveGenerator(pieceLocations.allyKing, pieceLocations.empty);
			kingMoves &= ~enemyDestSquares;

			if constexpr (DrawingBitboards) {
				drawBitboardImage(getDefaultColorGetter(kingMoves.all()), "king_moves.png");
				drawBitboardImage(getDefaultColorGetter(pieceLocations.allyKing, { 0, 255, 0 }), "ally_king.png");
			}

			bool inCheck = (enemyDestSquares & pieceLocations.allyKing);
			auto isCastlingClear = [&](const auto& castleMove) {
				auto squaresClear = (castleMove.squaresBetweenRookAndKing & ~pieceLocations.empty) == 0;
				auto squaresNotChecked = (castleMove.squaresBetweenRookAndKing & enemyDestSquares) == 0;
				return (!inCheck && squaresClear && squaresNotChecked);
			};
			if (!inCheck) {
				if (turnData.allies.canCastleKingside() && isCastlingClear(turnData.allyKingside)) {
					kingMoves.emptyDestSquares |= makeBitboard(turnData.allyKingside.kingTo);
				}
				if (turnData.allies.canCastleQueenside() && isCastlingClear(turnData.allyQueenside)) {
					kingMoves.emptyDestSquares |= makeBitboard(turnData.allyQueenside.kingTo);
				}
			} else {
				kingMoves &= ~attackerData.indirectRays;
			}
			addMoves(moves, nextSquare(pieceLocations.allyKing), kingMoves, King, pieceLocations, turnData.enemies, DEFAULT_MOVE_ADDER);

			return kingMoves.all();
		}

		static void addEnPassantMoves(MoveVector& moves, Bitboard pawns, Square jumpedEnemyPawn) {
			ProfilerLock l{ getEnPessantProfiler() };

			auto enPessantData = allyPawnAttackGenerator(pawns, jumpedEnemyPawn);
			if (!enPessantData.pawns) {
				return;
			}
			auto from = Square::None;
			while (nextSquare(enPessantData.pawns, from)) {
				moves.emplace_back(from, enPessantData.squareInFrontOfEnemyPawn, jumpedEnemyPawn, Pawn, Pawn, Piece::None);
			}
		}

		static Bitboard addPinMoves(MoveVector& moves, Piece pieceType, Bitboard pieceLocations, const PinMap& pinMap) {
			auto movablePieces = pieceLocations;

			auto currSquare = Square::None;
			while (nextSquare(pieceLocations, currSquare)) {
				if (pinMap[currSquare].cannotMove) {
					movablePieces &= ~makeBitboard(currSquare);
				} else if (pinMap[currSquare].mustCapturePinner) {
					movablePieces &= ~makeBitboard(currSquare);
					Move move{ currSquare, pinMap[currSquare].pinnerSquare, pieceType, pinMap[currSquare].pinnerPieceType };
					moves.push_back(move);
				}
			}
			return movablePieces;
		}

		static PositionData calcAllLegalMoves(const Position::ImmutableTurnData& turnData) {
			MoveVector moves;

			auto& allies = turnData.allies;
			auto& enemies = turnData.enemies;
			
			PieceLocationData pieceLocations{ allies[King], allies.calcAllLocations(), enemies.calcAllLocations() };
			if constexpr (DrawingBitboards) {
				drawPieceLocations(pieceLocations,"normal_piece_locations.png");
			}

			auto enemyDestSquares = calcEnemyDestSquares(enemies, pieceLocations);
			auto allyDestSquares = 0_bb;

			auto kingAttackerData = calcAttackers(White, enemies, pieceLocations.empty, pieceLocations.allyKing);
			allyDestSquares |= addKingMoves(moves, turnData, pieceLocations, enemyDestSquares, kingAttackerData);
			 
			if (kingAttackerData.hasMultipleAttackers()) { //if there are multiple checks, we have to move the king
				return { moves, moves.empty() };
			}

			auto attackedAllies = enemyDestSquares & pieceLocations.allies;

			auto slidingAttackers = kingAttackerData.attackers[Queen] | kingAttackerData.attackers[Rook] | kingAttackerData.attackers[Bishop];
			auto pinMap = calcPinnedAllies(allies, enemies, attackedAllies, slidingAttackers, pieceLocations);
			
			auto addMovesImpl = [&](Piece piece, auto moveGenerator) -> Bitboard {
				auto movablePieces = addPinMoves(moves, piece, allies[piece], pinMap);
				allyDestSquares |= addMoves(moves, movablePieces, piece, pieceLocations, enemies, moveGenerator, kingAttackerData);
				return movablePieces;
			};

			addMovesImpl(Queen, queenMoveGenerator);
			addMovesImpl(Rook, rookMoveGenerator);
			addMovesImpl(Bishop, bishopMoveGenerator);
			addMovesImpl(Knight, knightMoveGenerator);
			auto movablePawns = addMovesImpl(Pawn, allyPawnMoveGenerator);

			if (enemies.doubleJumpedPawn != Square::None) {
				addEnPassantMoves(moves, movablePawns, enemies.doubleJumpedPawn);
			}

			if constexpr (PROFILING) {
				auto& profiler = getLegalMoveGenerationProfiler().get();
				profiler.legalMovesGenerated += moves.size();
			}

			bool inCheck = (enemyDestSquares & pieceLocations.allyKing);
			bool checkmate = inCheck && moves.empty();

			Bitboard whiteSquares = 0;
			Bitboard blackSquares = 0;

			if constexpr (White) {
				whiteSquares = allyDestSquares;
				blackSquares = enemyDestSquares;
			} else {
				blackSquares = allyDestSquares;
				whiteSquares = enemyDestSquares;
			}
			return { std::move(moves), whiteSquares, blackSquares, inCheck, checkmate };
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