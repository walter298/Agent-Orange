module Chess.Position:PositionObject;

import Chess.Position.RepetitionMap;

import Chess.Assert;
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
        //if we can't castle eitherway or we're just not moving the king, don't try to castle
        if ((!turnData.allies.canCastleKingside() && !turnData.allies.canCastleQueenside()) || move.movedPiece != King) {
            return false;
        }

        //as soon as we move the king, we can't castle anymore
        turnData.allies.disallowQueensideCastling();
        turnData.allies.disallowKingsideCastling();

        if (turnData.allyKingside.kingTo == move.to) {
            moveSquare(turnData.allies[King], move.from, turnData.allyKingside.kingTo);
            moveSquare(turnData.allies[Rook], turnData.allyKingside.rookFrom, turnData.allyKingside.rookTo);
            return true;
        }
        else if (turnData.allyQueenside.kingTo == move.to) {
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
        }
        else {
            addSquare(pawns, move.to);
        }
    }

    void capturePiece(const Position::MutableTurnData& turnData, const Move& move) {
        if (move.capturedPiece == Rook) {
            if (move.to == turnData.enemyKingside.rookFrom) {
                turnData.enemies.disallowKingsideCastling();
            }
            else if (move.to == turnData.enemyQueenside.rookFrom) {
                turnData.enemies.disallowQueensideCastling();
            }
        }
        if (move.capturedPawnSquareEnPassant == Square::None) {
            removeSquare(turnData.enemies[move.capturedPiece], move.to);
        }
        else {
            removeSquare(turnData.enemies[move.capturedPiece], move.capturedPawnSquareEnPassant);
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
        }
        else {
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

        repetition::push(*this);
    }

    bool isEnPessant(const Move& move) {
        if (move.movedPiece != Pawn || move.capturedPiece != Piece::None) {
            return false;
        }
        auto fromFile = calcFile(fileOf(move.from));
        auto destFile = calcFile(fileOf(move.to));
        return fromFile != destFile;
    }

    void Position::move(std::string_view moveStr) {
        zAssert(moveStr.size() == 4 || moveStr.size() == 5);

        auto move = Move::null();

        auto from = parseSquare(moveStr.substr(0, 2));
        auto to = parseSquare(moveStr.substr(2, 2));
        zAssert(from && to);
        move.from = *from;
        move.to = *to;

        auto turnData = getTurnData();

        move.movedPiece = turnData.allies.findPiece(*from);
        zAssert(move.movedPiece != Piece::None);

        move.capturedPiece = turnData.enemies.findPiece(*to);

        //detect en passant capture
        if (isEnPessant(move)) {
            move.capturedPawnSquareEnPassant = turnData.enemies.doubleJumpedPawn;
            move.capturedPiece = Pawn;
        }

        if (moveStr.size() == 5) { //if there is a pawn promotion
            move.promotionPiece = parsePiece(moveStr[4]);
        }
        this->move(move);
    }

    bool operator==(const Position& a, const Position& b) {
        auto aTurnData = a.getTurnData();
        auto bTurnData = b.getTurnData();
        auto sidesEqual = std::ranges::equal(aTurnData.allies, bTurnData.allies) &&
            std::ranges::equal(aTurnData.enemies, bTurnData.enemies);

        auto checkCastlingPrivileges = [](const PieceState& allies, const PieceState& otherAllies) {
           return allies.canCastleKingside() == otherAllies.canCastleKingside()
                && allies.hasCastledQueenside() == otherAllies.hasCastledQueenside()
                && allies.canCastleKingside() == otherAllies.canCastleKingside()
                && allies.hasCastledQueenside() == otherAllies.hasCastledQueenside();
        };
        auto areCastlingPrivilegesEqual = checkCastlingPrivileges(aTurnData.allies, bTurnData.allies) &&
            checkCastlingPrivileges(aTurnData.enemies, bTurnData.enemies);

        auto jumpedPawnsEqual = 
            (aTurnData.allies.doubleJumpedPawn == bTurnData.allies.doubleJumpedPawn) &&
            (aTurnData.enemies.doubleJumpedPawn == bTurnData.enemies.doubleJumpedPawn);

        return areCastlingPrivilegesEqual && sidesEqual && jumpedPawnsEqual;
    }
}