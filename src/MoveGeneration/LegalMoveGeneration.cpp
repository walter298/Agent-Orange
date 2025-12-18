module Chess.LegalMoveGeneration;

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
import :Magic;
import :PawnMoveGeneration;
import :PieceLocations;
import :ReverseAttackGenerator;

namespace chess {
	struct Checklines {
		Bitboard kingAttackers = 0;
		Bitboard squares = 0;
		bool multipleChecks = false;
	};

	struct EnemyMoveData {
		Bitboard squares = 0;
		Checklines checklines;
	};

	template<typename T>
	concept MoveAdder = std::invocable<T, std::vector<Move>&, Move>;

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

		static constexpr auto PAWN_ADDER = [](std::vector<Move>& moves, Move move) {
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

		static constexpr auto DEFAULT_MOVE_ADDER = [](std::vector<Move>& moves, const Move& move) {
			moves.push_back(move);
		};

		template<typename MoveGenerator>
		static MoveGen invokeAttackGenerator(Bitboard movingEnemies, Bitboard empty, MoveGenerator attackGen) {
			if constexpr (PawnMoveGenerator<MoveGenerator>) {
				return attackGen(movingEnemies, ALL_SQUARES);
			} else {
				return attackGen(movingEnemies, empty);
			}
		}

		template<typename MoveGenerator>
		static MoveGen calcReverseAttacks(const PieceLocationData& pieceLocations, MoveGenerator) {
			constexpr auto reverseAttackGenerator = ReverseAttackGenerator<MoveGenerator>::get();
			return invokeAttackGenerator(pieceLocations.allyKing, pieceLocations.empty, reverseAttackGenerator);
		}

		template<typename MoveGenerator>
		static void calcChecklines(Checklines& checkLines, Bitboard possibleAttackers, const PieceLocationData& pieceLocations, 
			MoveGenerator enemySquareCalculator)
		{
			auto checklineFromKing = calcReverseAttacks(pieceLocations, enemySquareCalculator);

			auto currAttacker = Square::None;
			auto attackers = checklineFromKing.nonEmptyDestSquares & possibleAttackers;
			checkLines.kingAttackers |= attackers;

			while (nextSquare(attackers, currAttacker)) {
				if (checkLines.squares) {
					checkLines.multipleChecks = true;
				}

				auto attackerBoard = makeBitboard(currAttacker);
				auto checklineFromAttacker = enemySquareCalculator(attackerBoard, pieceLocations.empty);
				checkLines.squares |= (checklineFromKing.all() & checklineFromAttacker.all());
				checkLines.squares |= attackerBoard;
			}
		}

		template<typename EnemySquareCalculator>
		static void calcEnemyMovesImpl(EnemyMoveData& moveData, Bitboard movingEnemies, const PieceLocationData& pieceLocations,
			EnemySquareCalculator enemyMoveCalculator)
		{
			auto enemySquares = invokeAttackGenerator(movingEnemies, pieceLocations.empty, enemyMoveCalculator);
			
			if (enemySquares.nonEmptyDestSquares & pieceLocations.allyKing) { //if enemy piece is checking the king
				calcChecklines(moveData.checklines, movingEnemies, pieceLocations, enemyMoveCalculator);
			}
			moveData.squares |= enemySquares.all();
		}

		static EnemyMoveData calcEnemyMoves(const PieceState& enemies, const PieceLocationData& pieceLocations) {
			ProfilerLock l{ getEnemyMoveProfiler() };

			EnemyMoveData ret;
			ret.squares |= kingMoveGenerator(enemies[King], pieceLocations.empty).all();

			calcEnemyMovesImpl(ret, enemies[Queen] | enemies[Bishop], pieceLocations, bishopMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Queen] | enemies[Rook], pieceLocations, rookMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Knight], pieceLocations, knightMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Pawn], pieceLocations, enemyPawnAttackGenerator);

			return ret;
		}

		static void drawEnemyLayoutBitboard(const PieceLocationData& pieceLocations,
			const EnemyMoveData& enemyMoveData)
		{
			auto colorGetter = [&](Bitboard bit) -> RGB {
//				if (bit & enemyMoveData.checklines.kingAttackers) {
//					return PINK;
//				}
				if (pieceLocations.allies & bit) {
					if (bit & enemyMoveData.squares) { 
						return RED; //ally under attack
					} else {
						return GREEN; //ally not under attack
					}
				}
				if (pieceLocations.enemies & bit) {
					if (bit & enemyMoveData.squares) {
						return PURPLE; //defended enemy piece
					}
					return BROWN; //undefended enemy piece
				}
				if (enemyMoveData.squares & bit) {
					return YELLOW; //empty square under attack
				}
				return WHITE; //empty square not under attack
			};
			drawBitboardImage(colorGetter, "enemy_layout_bitboard.png");
		}

		static void drawEnemyAttackBitboard(const PieceLocationData& pieceLocations, 
			const EnemyMoveData& enemyMoveData)
		{
			constexpr RGB ATTACKER_COLOR{ 255, 0, 0 };
			constexpr RGB CHECKLINE_COLOR{ 97, 10, 255 };
			constexpr RGB EMPTY_COLOR{ 255, 255, 0 };
			constexpr RGB ALLY_KING_COLOR{ 0, 255, 0 };

			auto colorGetter = [&](Bitboard bit) -> RGB {
				if (pieceLocations.allyKing & bit) {
					return ALLY_KING_COLOR;
				} else if (enemyMoveData.checklines.kingAttackers & bit) {
					return ATTACKER_COLOR;
				} else if (enemyMoveData.checklines.squares & bit) {
					return CHECKLINE_COLOR;
				} else {
					return EMPTY_COLOR;
				}
			};
			drawBitboardImage(colorGetter, "enemy_attack_bitboard.png");
		}

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

		static Bitboard getPinnedAllies(const PieceState& enemies, const EnemyMoveData& originalEnemyMoveData, 
			const PieceLocationData& pieceLocations)
		{
			ProfilerLock l{ getCalcPinnedAlliesProfiler() };

			auto isPinned = [&](Square allySquare) {
				PieceLocationData pieceLocationsWithoutAlly{
					pieceLocations.allyKing, pieceLocations.allies & ~makeBitboard(allySquare), pieceLocations.enemies
				};
				auto enemyMoveData = calcEnemyMoves(enemies, pieceLocationsWithoutAlly);
				return enemyMoveData.checklines.squares & ~originalEnemyMoveData.checklines.squares;
			};

			Bitboard pinnedAllies = 0;
			auto attackedAllies = originalEnemyMoveData.squares & pieceLocations.allies;
			auto currSquare = Square::None;
			while (nextSquare(attackedAllies, currSquare)) {
				if (isPinned(currSquare)) {
					pinnedAllies |= makeBitboard(currSquare);
				}
			}
			return pinnedAllies;
		}

		static MoveGen calcLegalKingMovesNoCheck(const PieceLocationData& pieceLocations, Bitboard enemyDestSquares) {
			auto ret = kingMoveGenerator(pieceLocations.allyKing, pieceLocations.empty);
			ret &= ~enemyDestSquares;
			return ret;
		}

		static Bitboard calcIndirectlyCheckedSquares(const PieceState& enemies, const PieceLocationData& pieceLocations) {
			Bitboard ret = 0;
			auto empty = pieceLocations.empty | pieceLocations.allyKing; //pretend that the ally king is not on the board

			ret |= rookMoveGenerator(enemies[Queen] | enemies[Rook], empty).all();
			ret |= bishopMoveGenerator(enemies[Queen] | enemies[Bishop], empty).all();

			return ret;
		}

		template<MoveAdder MoveAdder>
		static void addMoves(std::vector<Move>& moves, Square piecePos, MoveGen destSquares, Piece pieceType,
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
		static Bitboard addMoves(std::vector<Move>& moves, Bitboard movablePieces, Piece pieceType, const PieceLocationData& pieceLocations,
			const PieceState& enemies, MoveGenerator moveGen, Bitboard checklines)
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

				if (checklines) {
					destSquares &= checklines;
				}
				if constexpr (PawnMoveGenerator<MoveGenerator>) {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, PAWN_ADDER);
				} else {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, DEFAULT_MOVE_ADDER);
				}
			}
			return allSquares;
		}

		static Bitboard addKingMoves(std::vector<Move>& moves, const Position::ImmutableTurnData& turnData, 
			const PieceLocationData& pieceLocations, Bitboard enemyMoves)
		{
			bool inCheck = (enemyMoves & pieceLocations.allyKing);
			auto isCastlingClear = [&](const auto& castleMove) {
				return (!inCheck && 
					castleMove.squaresBetweenRookAndKing == 0 && 
					(castleMove.squaresBetweenRookAndKing & enemyMoves));
			};
			auto kingMoves = calcLegalKingMovesNoCheck(pieceLocations, enemyMoves);
			if constexpr (DrawingBitboards) {
				drawBitboardImage(getDefaultColorGetter(kingMoves.all()), "king_moves.png");
				drawBitboardImage(getDefaultColorGetter(pieceLocations.allyKing, { 0, 255, 0 }), "ally_king.png");
			}

			if (inCheck) {
				kingMoves &= ~calcIndirectlyCheckedSquares(turnData.enemies, pieceLocations);
			} else { //if the king is not in check, then we might be able to castle
				if (turnData.allies.canCastleKingside() && isCastlingClear(turnData.allyKingside)) {
					kingMoves.emptyDestSquares |= turnData.allyKingside.kingTo;
				}
				if (turnData.allies.canCastleQueenside() && isCastlingClear(turnData.allyQueenside)) {
					kingMoves.emptyDestSquares |= turnData.allyQueenside.kingTo;
				}
			}
			addMoves(moves, nextSquare(pieceLocations.allyKing), kingMoves, King, pieceLocations, turnData.enemies, DEFAULT_MOVE_ADDER);

			return kingMoves.all();
		}

		static void addEnPassantMoves(std::vector<Move>& moves, Bitboard pawns, Square jumpedEnemyPawn) {
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

		static PositionData calcAllLegalMoves(const Position::ImmutableTurnData& turnData) {
			std::vector<Move> moves;

			constexpr auto AVERAGE_MOVES_PER_POSITION = 40uz;
			moves.reserve(AVERAGE_MOVES_PER_POSITION); 

			auto& allies = turnData.allies;
			auto& enemies = turnData.enemies;
			
			PieceLocationData pieceLocations{ allies[King], allies.calcAllLocations(), enemies.calcAllLocations() };
			if constexpr (DrawingBitboards) {
				drawPieceLocations(pieceLocations,"normal_piece_locations.png");
			}

			const auto enemyMoveData = calcEnemyMoves(enemies, pieceLocations);
			if constexpr (DrawingBitboards) {
				drawEnemyAttackBitboard(pieceLocations, enemyMoveData);
				drawEnemyLayoutBitboard(pieceLocations, enemyMoveData);
			}

			Bitboard allySquares = 0;

			/*Enemy destination squares are blocked by the king, so if the king in check, ensure that we are not moving
			onto a square that was blocked but is still indirectly checked*/
			allySquares |= addKingMoves(moves, turnData, pieceLocations, enemyMoveData.squares);

			if (enemyMoveData.checklines.multipleChecks) { //if there are multiple checks, we have to move the king
				return { moves, moves.empty() };
			}

			auto pinnedAllies = getPinnedAllies(enemies, enemyMoveData, pieceLocations);
			auto checklines = enemyMoveData.checklines.squares;

			auto addMovesImpl = [&](Piece piece, auto moveGenerator) {
				auto movablePieces = allies[piece] & ~pinnedAllies;
				allySquares |= addMoves(moves, movablePieces, piece, pieceLocations, enemies, moveGenerator, checklines);
			};
			addMovesImpl(Queen, queenMoveGenerator);
			addMovesImpl(Rook, rookMoveGenerator);
			addMovesImpl(Bishop, bishopMoveGenerator);
			addMovesImpl(Knight, knightMoveGenerator);
			addMovesImpl(Pawn, allyPawnMoveGenerator);

			if (enemies.doubleJumpedPawn != Square::None) {
				addEnPassantMoves(moves, allies[Pawn] & ~pinnedAllies, enemies.doubleJumpedPawn);
			}

			if constexpr (PROFILING) {
				auto& profiler = getLegalMoveGenerationProfiler().get();
				profiler.legalMovesGenerated += moves.size();
			}

			bool inCheck = (enemyMoveData.squares & pieceLocations.allyKing);
			bool checkmate = inCheck && moves.empty();

			Bitboard whiteSquares = 0;
			Bitboard blackSquares = 0;

			if constexpr (White) {
				whiteSquares = allySquares;
				blackSquares = enemyMoveData.squares;
			} else {
				blackSquares = allySquares;
				whiteSquares = enemyMoveData.squares;
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
