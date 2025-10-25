module Chess.LegalMoveGeneration;

import std;
import Chess.PieceMap;
import Chess.RankCalculator;
import :ChainedMoveGenerator;
import :KingMoveGeneration;
import :KnightMoveGeneration;
import :NonPawnMoveGenerator;
import :PawnMoveGeneration;
import :PieceLocations;
import :SlidingMoveGeneration;

namespace chess {
	struct Checklines {
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
			 typename EnemyPawnMoveGenerator, typename EnemyPawnAttackGenerator, Bitboard PromotionRank>
	struct MoveGeneratorImpl {
		static inline AllyPawnMoveGenerator allyPawnMoveGenerator;
		static inline AllyPawnAttackGenerator allyPawnAttackGenerator;
		static inline EnemyPawnMoveGenerator enemyPawnMoveGenerator;
		static inline EnemyPawnAttackGenerator enemyPawnAttackGenerator;

		static constexpr auto PAWN_ADDER = [](std::vector<Move>& moves, Move move) {
			if (makeBitboard(move.to) & PromotionRank) {
				constexpr std::array PROMOTION_PIECES{ Queen, Rook, Bishop, Knight };
				for (auto piece : PROMOTION_PIECES) {
					move.promotedPiece = piece;
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

		template<typename PawnAttackGenerator>
		static auto wrapPawnAttackGenerator(PawnAttackGenerator pawnAttackGenerator, Bitboard enemies) {
			return [=](Bitboard pawns, Bitboard empty) {
				return pawnAttackGenerator(pawns, enemies);
			};
		}

		template<typename MoveGenerator>
		static MoveGen calcReverseAttacks(const PieceLocationData& pieceLocations, MoveGenerator moveGenerator) {
			if constexpr (NonPawnMoveGenerator<MoveGenerator>) {
				return moveGenerator(pieceLocations.allyKing, pieceLocations.empty);
			} else {
				return allyPawnAttackGenerator(pieceLocations.allyKing, pieceLocations.enemies);
			}
		}

		template<typename MoveGenerator>
		static void calcChecklines(Checklines& checkLines, Bitboard possibleAttackers, const PieceLocationData& pieceLocations, 
			MoveGenerator enemySquareCalculator)
		{
			auto checklineFromKing = calcReverseAttacks(pieceLocations, enemySquareCalculator);

			auto currAttacker = Square::None;
			auto attackers = checklineFromKing.nonEmptyDestSquares & possibleAttackers;
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
			EnemyMoveData ret;
			ret.squares |= kingMoveGenerator(enemies[King], pieceLocations.empty).all();

			calcEnemyMovesImpl(ret, enemies[Queen] | enemies[Bishop], pieceLocations, diagonalMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Queen] | enemies[Rook], pieceLocations, orthogonalMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Knight], pieceLocations, knightMoveGenerator);
			calcEnemyMovesImpl(ret, enemies[Pawn], pieceLocations, wrapPawnAttackGenerator(enemyPawnAttackGenerator, ALL_SQUARES));

			return ret;
		}

		static Bitboard getPinnedAllies(const PieceState& enemies, Bitboard attackedAllies, const PieceLocationData& pieceLocations, bool isWhite) {
			PieceLocationData pieceLocationsWithoutAttackedAllies{
				pieceLocations.allyKing, pieceLocations.allies & ~attackedAllies, pieceLocations.enemies
			};
			auto enemyMoves = calcEnemyMoves(enemies, pieceLocationsWithoutAttackedAllies);

			return enemyMoves.checklines.squares & attackedAllies & ~pieceLocations.allyKing;
		}

		static Bitboard getPinnedAllies(const PieceState& enemies, Bitboard attackedAllies, const PieceLocationData& pieceLocations) {
			PieceLocationData pieceLocationsWithoutAttackedAllies{
				pieceLocations.allyKing, pieceLocations.allies & ~attackedAllies, pieceLocations.enemies
			};
			auto enemyMoves = calcEnemyMoves(enemies, pieceLocationsWithoutAttackedAllies);

			return enemyMoves.checklines.squares & attackedAllies & ~pieceLocations.allyKing;
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
				if constexpr (NonPawnMoveGenerator<MoveGenerator>) {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, DEFAULT_MOVE_ADDER);
				} else {
					addMoves(moves, currPiecePos, destSquares, pieceType, pieceLocations, enemies, PAWN_ADDER);
				}
			}
		}

		static void addKingMoves(std::vector<Move>& moves, const Position::ImmutableTurnData& turnData, 
			const PieceLocationData& pieceLocations, Bitboard enemyMoves)
		{
			auto isCastlingClear = [enemyMoves](const auto& castleMove) {
				return (castleMove.squaresBetweenRookAndKing == 0 && (castleMove.squaresBetweenRookAndKing & enemyMoves));
			};
			auto kingMoves = calcLegalKingMovesNoCheck(pieceLocations, enemyMoves);;
			if (enemyMoves & pieceLocations.allyKing) {
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
			auto enPessantData = allyPawnAttackGenerator(pawns, jumpedEnemyPawn);
			if (!enPessantData.pawns) {
				return;
			}
			auto from = Square::None;
			while (nextSquare(enPessantData.pawns, from)) {
				moves.emplace_back(from, enPessantData.destSquare, jumpedEnemyPawn, Pawn, Pawn, Piece::None);
			}
		}

		static std::vector<Move> calcAllLegalMoves(const Position::ImmutableTurnData& turnData) {
			std::vector<Move> moves;
			auto& allies = turnData.allies;
			auto& enemies = turnData.enemies;

			PieceLocationData pieceLocations{ allies[King], allies.calcAllLocations(), enemies.calcAllLocations() };

			auto enemyMoveData = calcEnemyMoves(enemies, pieceLocations);

			/*Enemy destination squares are blocked by the king, so if the king in check, ensure that we are not moving
			onto a square that was blocked but is still indirectly checked*/
			addKingMoves(moves, turnData, pieceLocations, enemyMoveData.squares);
			if (enemyMoveData.checklines.multipleChecks) { //if there are multiple checks, we have to move the king
				return moves;
			}

			auto attackedAllies = enemyMoveData.squares & pieceLocations.allies;
			auto pinnedAllies = getPinnedAllies(enemies, attackedAllies, pieceLocations);
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
			
			return moves;
		}
	};

	std::vector<Move> calcAllLegalMoves(const Position& pos) {
		auto turnData = pos.getTurnData();
		
		if (turnData.isWhite) {
			using MoveGenerator = MoveGeneratorImpl<WhitePawnMoveGenerator, WhitePawnAttackGenerator,
													BlackPawnMoveGenerator, BlackPawnAttackGenerator, calcRank<8>()>;
			return MoveGenerator::calcAllLegalMoves(turnData);
		} else {
			using MoveGenerator = MoveGeneratorImpl<BlackPawnMoveGenerator, BlackPawnAttackGenerator,
				WhitePawnMoveGenerator, WhitePawnAttackGenerator, calcRank<1>()>;
			return MoveGenerator::calcAllLegalMoves(turnData);
		}
	}
}
