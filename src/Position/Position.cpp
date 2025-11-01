module;

#include <cassert>

module Chess.Position;

import Chess.RankCalculator;
import :Parse;

namespace chess {
    void Position::setPos(const PositionCommand& positionCommand) {
        m_whitePieces.clear();
        m_blackPieces.clear();

        parseBoard(positionCommand.board, m_whitePieces, m_blackPieces);
        m_isWhiteMoving = (positionCommand.color == 'w');
        parseCastlingPrivileges(positionCommand.castlingPrivileges, m_whitePieces, m_blackPieces);
        parseEnPessantSquare(positionCommand.enPessantSquare, m_isWhiteMoving, m_isWhiteMoving ? m_blackPieces : m_whitePieces);
        
        for (const auto& moveStr : positionCommand.moves) {
            move(moveStr);
		}
    }

    bool tryCastle(Position::MutableTurnData& turnData, const Move& move) {
        if ((!turnData.allies.canCastleKingside() && !turnData.allies.canCastleQueenside()) || move.movedPiece != King) {
            return false;
        }
        
        turnData.allies.disallowQueensideCastling();
        turnData.allies.disallowKingsideCastling();

        if (turnData.allyKingside.kingTo == move.to) {
            moveSquare(turnData.allies[King], move.from, turnData.allyKingside.kingTo);
            moveSquare(turnData.allies[Rook], turnData.allyKingside.rookFrom, turnData.allyKingside.rookTo);
            return true;
        } else if (turnData.allyQueenside.kingTo == move.to) {
            moveSquare(turnData.allies[King], move.from, turnData.allyQueenside.kingTo);
            moveSquare(turnData.allies[Rook], turnData.allyQueenside.rookFrom, turnData.allyQueenside.rookTo);
            return true;
        }
        return false;
    }

    bool isPawnDoubleJump(const Position::MutableTurnData& turnData, const Move& move) {
        return (makeBitboard(move.from) & turnData.allyPawnRank) && (makeBitboard(move.to) & turnData.jumpedAllyPawnRank);
    }

    void movePawn(const Position::MutableTurnData& turnData, const Move& move, Bitboard& pawns) {
		if (isPawnDoubleJump(turnData, move)) {
            turnData.allies.doubleJumpedPawn = move.to;
		}
    	if (move.promotionPiece != Piece::None) {
            addSquare(turnData.allies[move.promotionPiece], move.to);
        } else {
            addSquare(pawns, move.to);
        }
    }

    void capturePiece(const Position::MutableTurnData& turnData, const Move& move) {
        if (move.capturedPiece == Rook) {
            if (move.to == turnData.enemyKingside.rookFrom) {
                turnData.enemies.disallowKingsideCastling();
            } else if (move.to == turnData.enemyQueenside.rookFrom) {
                turnData.enemies.disallowQueensideCastling();
            }
        }
		if (move.enPessantSquare == Square::None) {
            removeSquare(turnData.enemies[move.capturedPiece], move.to);
		} else {
            removeSquare(turnData.enemies[move.capturedPiece], move.enPessantSquare);
		}
    }

    void normalMove(const Position::MutableTurnData& turnData, const Move& move) {
        //move the piece (destination square handled with pawn promotions)
        auto& movedPiecePos = turnData.allies[move.movedPiece];
        removeSquare(turnData.allies[move.movedPiece], move.from);

        //capture the piece!
        if (move.capturedPiece != Piece::None) {
            capturePiece(turnData, move);
        }

        turnData.allies.doubleJumpedPawn = Square::None;
        turnData.enemies.doubleJumpedPawn = Square::None;

        if (move.movedPiece == Pawn) {
            movePawn(turnData, move, turnData.allies[Pawn]);
        } else {
            addSquare(movedPiecePos, move.to);
        }
    }

	void Position::move(const Move& move) {
        auto turnData = getTurnData();
        if (move.movedPiece == Rook) {
	        if (move.from == turnData.allyKingside.rookFrom) {
                turnData.allies.disallowKingsideCastling();
	        } else if (move.from == turnData.allyQueenside.rookFrom) {
                turnData.allies.disallowQueensideCastling();
	        }
        }
		if (!tryCastle(turnData, move)) {
            normalMove(turnData, move);
		}
        m_isWhiteMoving = !m_isWhiteMoving; //alternate turns
	}

    bool isEnPessant(const Move& move) {
	    if (move.movedPiece != Piece::None || move.capturedPiece != Piece::None) {
            return false;
	    }
		auto fromFile = calcFile(fileOf(move.from));
        auto destFile = calcFile(fileOf(move.to));
        return fromFile != destFile;
    }

    void Position::move(std::string_view moveStr) { 
        assert(moveStr.size() == 4 || moveStr.size() == 5);

        auto move = Move::null();

    	auto from = parseSquare(moveStr.substr(0, 2));
        auto to   = parseSquare(moveStr.substr(2, 2));
        assert(from && to);
        move.from = *from;
        move.to   = *to;

        auto turnData = getTurnData();

        move.movedPiece = turnData.allies.findPiece(*from);
        assert(move.movedPiece != Piece::None);
        
        move.capturedPiece = turnData.enemies.findPiece(*to);

		//detect en passant capture
        if (isEnPessant(move)) {
            move.enPessantSquare = turnData.enemies.doubleJumpedPawn;
            move.capturedPiece = Pawn;
        }

        if (moveStr.size() == 5) { //if there is a pawn promotion
            move.promotionPiece = parsePiece(moveStr[4]);
        }
        this->move(move);
	}
}