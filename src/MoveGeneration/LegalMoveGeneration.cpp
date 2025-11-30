module;

#include <cassert>

module Chess.LegalMoveGeneration;

import nlohmann.json;

import std;

import Chess.BitboardImage;
import Chess.PieceMap;
import Chess.Profiler;
import Chess.RankCalculator;
import :ChainedMoveGenerator;
import :KingMoveGeneration;
import :KnightMoveGeneration;
import :NonPawnMoveGenerator;
import :PawnMoveGeneration;
import :PieceLocations;
import :ReverseAttackGenerator;
import :SlidingMoveGeneration;

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

	template<typename AllyPawnMoveGenerator, typename AllyPawnAttackGenerator,
			 typename EnemyPawnMoveGenerator, typename EnemyPawnAttackGenerator, Bitboard PromotionRank, 
			 bool DrawingBitboards>
	struct MoveGeneratorImpl {
		static inline AllyPawnMoveGenerator allyPawnMoveGenerator;
		static inline AllyPawnAttackGenerator allyPawnAttackGenerator;
		static inline EnemyPawnMoveGenerator enemyPawnMoveGenerator;
		static inline EnemyPawnAttackGenerator enemyPawnAttackGenerator;

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

		template<typename PawnMoveGenerator>
		static auto wrapPawnMoveGenerator(PawnMoveGenerator pawnMoveGen, Bitboard enemies)
		{
			return [=](Bitboard pawns, Bitboard empty) {
				return pawnMoveGen(pawns, empty, enemies);
			};
		}

		/*template<typename PawnAttackGenerator>
		static auto wrapPawnAttackGenerator(PawnAttackGenerator pawnAttackGenerator, Bitboard enemies) {
			return [=](Bitboard pawns, Bitboard empty) {
				return pawnAttackGenerator(pawns, enemies);
			};
		}*/

		template<typename MoveGenerator>
		static MoveGen calcReverseAttacks(const PieceLocationData& pieceLocations, MoveGenerator moveGenerator) {
			constexpr auto reverseAttackGenerator = ReverseAttackGenerator<MoveGenerator>::get();
			if constexpr (PawnMoveGenerator<MoveGenerator>) {
				return reverseAttackGenerator(pieceLocations.allyKing, pieceLocations.enemies);
			} else {
				return reverseAttackGenerator(pieceLocations.allyKing, pieceLocations.empty);
			}
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
			auto moves = enemyMoveCalculator(movingEnemies, pieceLocations.empty);
			if (moves.nonEmptyDestSquares & pieceLocations.allyKing) {
				calcChecklines(moveData.checklines, movingEnemies, pieceLocations, enemyMoveCalculator);
			}
			moveData.squares |= moves.all();
		}

		static EnemyMoveData calcEnemyMoves(const PieceState& enemies, const PieceLocationData& pieceLocations) {
			ProfilerLock l{ getEnemyMoveProfiler() };

			EnemyMoveData ret;
			ret.squares |= kingMoveGenerator(enemies[King], pieceLocations.empty).all();

			calcEnemyMovesImpl(ret, enemies[Queen] | enemies[Bishop], pieceLocations, diagonalMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Queen] | enemies[Rook], pieceLocations, orthogonalMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Knight], pieceLocations, knightMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Pawn], pieceLocations, enemyPawnAttackGenerator);

			return ret;
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
			constexpr RGB ALLY_COLOR{ 0, 0, 255 };
			constexpr RGB ENEMY_COLOR{ 255, 0, 0 };
			constexpr RGB EMPTY_COLOR{ 255, 255, 0 };
			auto colorGetter = [&](Bitboard bit) -> RGB {
				if (pieceLocations.allies & bit) {
					return ALLY_COLOR;
				} else if (pieceLocations.enemies & bit) {
					return ENEMY_COLOR;
				} else {
					return EMPTY_COLOR;
				}
			};
			drawBitboardImage(colorGetter, filename);
		}

		static Bitboard getPinnedAllies(const PieceState& enemies, Bitboard actualChecklines, const PieceLocationData& pieceLocations)
		{
			Bitboard pinnedAllies = 0;
			
			auto getPinnedAlliesImpl = [&](Bitboard movingEnemies, const auto& moveGenerator) {
				EnemyMoveData directionalAttackData;
				calcEnemyMovesImpl(directionalAttackData, movingEnemies, pieceLocations, moveGenerator);
				auto attackedAllies = directionalAttackData.squares & pieceLocations.allies;
				if (attackedAllies == 0) { //there can be no pins if there are no attacked allies
					return;
				}

				//pretend that the attacked allies are not on the board to reveal any checklines running through them
				PieceLocationData pieceLocationsWithAttackedAllies{
					pieceLocations.allyKing,
					(pieceLocations.allies & ~attackedAllies) | pieceLocations.allyKing, //OR in case of check
					pieceLocations.enemies
				};
				EnemyMoveData directionalTransparentAttackData;
				calcEnemyMovesImpl(directionalTransparentAttackData, movingEnemies, pieceLocationsWithAttackedAllies,
									moveGenerator);
				auto newChecklines = directionalTransparentAttackData.checklines.squares & ~actualChecklines; //remove original checklines
				pinnedAllies |= (newChecklines & attackedAllies & ~pieceLocations.allyKing);
			};

			//calculate diagonal pins
			getPinnedAlliesImpl(enemies[Queen] | enemies[Bishop], northEastSlidingMoveGenerator);
			getPinnedAlliesImpl(enemies[Queen] | enemies[Bishop], northWestSlidingMoveGenerator);
			getPinnedAlliesImpl(enemies[Queen] | enemies[Bishop], southEastSlidingMoveGenerator);
			getPinnedAlliesImpl(enemies[Queen] | enemies[Bishop], southWestSlidingMoveGenerator);

			//calculate orthogonal pins
			getPinnedAlliesImpl(enemies[Queen] | enemies[Rook], northSlidingMoveGenerator);
			getPinnedAlliesImpl(enemies[Queen] | enemies[Rook], eastSlidingMoveGenerator);
			getPinnedAlliesImpl(enemies[Queen] | enemies[Rook], southSlidingMoveGenerator);
			getPinnedAlliesImpl(enemies[Queen] | enemies[Rook], westSlidingMoveGenerator);

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

			ret |= orthogonalMoveGenerator(enemies[Queen] | enemies[Rook], empty).all();
			ret |= diagonalMoveGenerator(enemies[Queen] | enemies[Bishop], empty).all();
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
		static void addMoves(std::vector<Move>& moves, Bitboard movablePieces, Piece pieceType, const PieceLocationData& pieceLocations,
			const PieceState& enemies, MoveGenerator moveGen, Bitboard checklines)
		{
			auto currPiecePos = Square::None;
			while (nextSquare(movablePieces, currPiecePos)) {
				auto destSquares = moveGen(makeBitboard(currPiecePos), pieceLocations.empty);
				if (checklines) {
					destSquares &= checklines;
				}
				if constexpr (PawnMoveGenerator<MoveGenerator>) {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, PAWN_ADDER);
				} else {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, DEFAULT_MOVE_ADDER);
				}
			}
		}

		static void addKingMoves(std::vector<Move>& moves, const Position::ImmutableTurnData& turnData, 
			const PieceLocationData& pieceLocations, Bitboard enemyMoves)
		{
			bool inCheck = (enemyMoves & pieceLocations.allyKing);
			auto isCastlingClear = [&](const auto& castleMove) {
				return (!inCheck && 
					castleMove.squaresBetweenRookAndKing == 0 && 
					(castleMove.squaresBetweenRookAndKing & enemyMoves));
			};
			auto kingMoves = calcLegalKingMovesNoCheck(pieceLocations, enemyMoves);
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
		}

		static void addEnPessantMoves(std::vector<Move>& moves, Bitboard pawns, Square jumpedEnemyPawn) {
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

		static LegalMoves calcAllLegalMoves(const Position::ImmutableTurnData& turnData) {
			std::vector<Move> moves;
			auto& allies = turnData.allies;
			auto& enemies = turnData.enemies;
			
			PieceLocationData pieceLocations{ allies[King], allies.calcAllLocations(), enemies.calcAllLocations() };
			if constexpr (DrawingBitboards) {
				drawPieceLocations(pieceLocations,"normal_piece_locations.png");
			}

			auto enemyMoveData = calcEnemyMoves(enemies, pieceLocations);
			if constexpr (DrawingBitboards) {
				drawEnemyAttackBitboard(pieceLocations, enemyMoveData);
			}

			/*Enemy destination squares are blocked by the king, so if the king in check, ensure that we are not moving
			onto a square that was blocked but is still indirectly checked*/
			addKingMoves(moves, turnData, pieceLocations, enemyMoveData.squares);
			if (enemyMoveData.checklines.multipleChecks) { //if there are multiple checks, we have to move the king
				return { moves, moves.empty() };
			}

			auto pinnedAllies = getPinnedAllies(enemies, enemyMoveData.checklines.squares, pieceLocations);
			auto checklines = enemyMoveData.checklines.squares;
			
			auto addMovesImpl = [&](Piece piece, auto moveGenerator) {
				auto movablePieces = allies[piece] & ~pinnedAllies;
				addMoves(moves, movablePieces, piece, pieceLocations, enemies, moveGenerator, checklines);
			};
			addMovesImpl(Queen, diagonalMoveGenerator | orthogonalMoveGenerator);
			addMovesImpl(Rook, orthogonalMoveGenerator);
			addMovesImpl(Bishop, diagonalMoveGenerator);
			addMovesImpl(Knight, knightMoveGenerator);
			addMovesImpl(Pawn, wrapPawnMoveGenerator(allyPawnMoveGenerator, pieceLocations.enemies));

			if (enemies.doubleJumpedPawn != Square::None) {
				addEnPessantMoves(moves, allies[Pawn] & ~pinnedAllies, enemies.doubleJumpedPawn);
			}

			if constexpr (PROFILING) {
				auto& profiler = getLegalMoveGenerationProfiler().get();
				profiler.legalMovesGenerated += moves.size();
			}

			bool inCheck = (enemyMoveData.squares & pieceLocations.allyKing);
			return { moves, inCheck && moves.empty() };
		}
	};

	template<bool DrawingBitboards>
	LegalMoves calcAllLegalMovesImpl(const Position& pos) {
		ProfilerLock l{ getLegalMoveGenerationProfiler() };

		auto turnData = pos.getTurnData();

		if (turnData.isWhite) {
			using MoveGenerator = MoveGeneratorImpl<WhitePawnMoveGenerator, WhitePawnAttackGenerator,
				BlackPawnMoveGenerator, BlackPawnAttackGenerator, calcRank<8>(), DrawingBitboards>;
			return MoveGenerator::calcAllLegalMoves(turnData);
		} else {
			using MoveGenerator = MoveGeneratorImpl<BlackPawnMoveGenerator, BlackPawnAttackGenerator,
				WhitePawnMoveGenerator, WhitePawnAttackGenerator, calcRank<1>(), DrawingBitboards>;
			return MoveGenerator::calcAllLegalMoves(turnData);
		}
	}

	LegalMoves calcAllLegalMoves(const Position& pos) {
		return calcAllLegalMovesImpl<false>(pos);
	}
	LegalMoves calcAllLegalMovesAndDrawBitboards(const Position& pos) {
		return calcAllLegalMovesImpl<true>(pos);
	}
}
