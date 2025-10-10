module;

#include <cstdlib>
#include <boost/parser/parser.hpp>

module Chess.Position;
import Chess.RankCalculator;

namespace chess {
    Piece PieceState::findPiece(Square square) const {
        auto board = makeBitboard(square);

        for (auto piece : ALL_PIECE_TYPES) {
            if ((*this)[piece] & board) {
                return piece;
            }
        }
        return Piece::None;
    }

    template<Bitboard PieceRow, Bitboard PawnRow>
    consteval PieceState makeSides() {
        PieceState ret;
        ret[Pawn] = PawnRow;
        ret[Rook] = makeBitboard(getSquare(PieceRow, 1)) | makeBitboard(getSquare(PieceRow, 8));
        ret[Knight] = makeBitboard(getSquare(PieceRow, 2)) | makeBitboard(getSquare(PieceRow, 7));
        ret[Bishop] = makeBitboard(getSquare(PieceRow, 3)) | makeBitboard(getSquare(PieceRow, 6));
        ret[Queen] = makeBitboard(getSquare(PieceRow, 4));
        ret[King] = makeBitboard(getSquare(PieceRow, 5));
        return ret;
    }

    Position Position::startPos() {
        Position ret;
        ret.m_whitePieces = makeSides<calcRank<1>(), calcRank<2>()>();
        ret.m_blackPieces = makeSides<calcRank<8>(), calcRank<7>()>();
        ret.m_isWhiteMoving = true;
        return ret;
    }

	Position Position::fromStartPos(std::string_view str) {
        auto ret = startPos();

        std::vector<std::string> moves;

        namespace bp = boost::parser;
        auto field = +(bp::char_ - (bp::char_(' ') | bp::char_('"') | bp::eoi));
        auto movesParser = bp::lit("moves ") >> +(field >> (bp::lit(' ') | bp::eoi));
        auto movesRes = bp::parse(str, bp::omit[*(bp::char_ - bp::char_('m'))] >> -movesParser, moves);

        if (movesRes) {
	        for (const auto& move : moves) {
                ret.move(move);
	        }
        }
        return ret;
    }

	std::string getFENString(std::string_view pgnPath) {
		std::ifstream file{ pgnPath.data() }; //hopefully pgnPath is null-terminated!
		if (!file.is_open()) {
			std::println("Error: could not open {}", pgnPath);
			std::exit(EXIT_FAILURE);
		}

		std::string line;
		bool foundFENStr = false;
		while (std::getline(file, line)) {
			if (line.starts_with("[F")) { //we only care about the [FEN "..."] line
				foundFENStr = true;
				break;
			}
		}
		if (!foundFENStr) {
			std::println("Error: could not find FEN string in PGN file");
			std::exit(EXIT_FAILURE);
		}
		return line;
	}

    Piece parsePiece(char chr) {
        Piece piece = Piece::None;
        switch (chr) {
        case 'k':
            piece = King;
            break;
        case 'q':
            piece = Queen;
            break;
        case 'r':
            piece = Rook;
            break;
        case 'b':
            piece = Bishop;
            break;
        case 'n':
            piece = Knight;
            break;
        case 'p':
            piece = Pawn;
            break;
        default:
            std::println("Error: {} not yet implemented :(", chr);
            std::exit(EXIT_FAILURE);
        }
        return piece;
    }

    void parseRow(std::string_view row, int& squareCount,
        chess::PieceState& whitePieces, chess::PieceState& blackPieces)
    {
        auto setPiecePosition = [](chess::PieceState& pieces, char pieceChr, int squareCount) {
            addSquare(pieces[parsePiece(pieceChr)], static_cast<Square>(squareCount));
        };

        for (auto chr : row) {
            if (std::isdigit(static_cast<unsigned char>(chr))) {
                squareCount += (chr - '0'); //skip empty squares
            } else {
                if (std::islower(static_cast<unsigned char>(chr))) {
                    setPiecePosition(blackPieces, chr, squareCount);
                } else {
                    setPiecePosition(whitePieces, std::tolower(static_cast<unsigned char>(chr)), squareCount);
                }
                squareCount++; 
            }
        }
    }

    Position Position::fromFENString(std::string_view fen) {
        namespace bp = boost::parser;

        auto field = +(bp::char_ - (bp::char_(' ') | bp::char_('"')));
        auto movesParser = bp::lit("moves ") >> +(field >> (bp::lit(' ') | bp::eoi));
        
        auto fieldsRes = bp::parse(fen,
            -bp::lit("[FEN \"") >> field >> bp::lit(' ') >> field >> bp::lit(' ') >>
            field >> bp::lit(' ') >> field >> bp::omit[*(bp::char_ - bp::char_('m'))] >>
            -movesParser
        );
        if (!fieldsRes) {
            std::println("Error parsing FEN string");
            std::exit(EXIT_FAILURE);
        }

        Position ret;

        const auto& [board, color, castlingPrivileges, enPessantData, moves] = *fieldsRes;

        auto rowParser = +(bp::char_ - bp::char_('/'));
        auto rowsRes = bp::parse(board, +(rowParser >> -bp::lit('/')));
        if (!rowsRes) {
            std::println("Error: failed to parse rows from FEN string");
            std::exit(EXIT_FAILURE);
        }

        int squareCount = 56;
        for (const auto& row : *rowsRes) {
            parseRow(row, squareCount, ret.m_whitePieces, ret.m_blackPieces);
            squareCount -= 16;
        }

        ret.m_isWhiteMoving = (color[0] == 'w');
        if (moves) {
            for (const auto& moveStr : *moves) {
                ret.move(moveStr);
            }
        }

        return ret;
    }

	void Position::move(const Move& move) {
		auto [allies, enemies] = getSidesMutable();
        m_isWhiteMoving = !m_isWhiteMoving; //alternate turns

		//move the piece (destination square handled with pawn promotions)
        auto& movedPiecePos = allies[move.movedPiece];
        removeSquare(allies[move.movedPiece], move.from);
       
        //capture the piece!
        if (move.capturedPiece != Piece::None) {
            removeSquare(enemies[move.capturedPiece], move.to);
        }

        //check for pawn promotions
        if (move.promotedPiece != Piece::None) {
            addSquare(allies[move.promotedPiece], move.to);
        } else {
            addSquare(movedPiecePos, move.to);
        }
	}

    Square parseSquare(std::string_view squareStr) {
        auto file = squareStr[0] - 'a';
        auto rank = squareStr[1] - '1';
        auto index = (rank * 8) + file;
        return static_cast<Square>(index);
    }

    void Position::move(std::string_view moveStr) { //todo: handle pawn promotions (don't yet know string format for them)
        assert(moveStr.size() == 4 || moveStr.size() == 5);

    	auto from = parseSquare(moveStr.substr(0, 2));
        auto to   = parseSquare(moveStr.substr(2, 2));

        auto [allies, enemies, isWhite] = getTurnSides();
        
        auto movedPiece    = allies.findPiece(from);
        auto capturedPiece = enemies.findPiece(to);
        auto promotedPiece = Piece::None;
        if (moveStr.size() == 5) { //if there is a pawn promotion
            promotedPiece = parsePiece(moveStr[4]);
        }
        move({ from, to, movedPiece, capturedPiece, promotedPiece });
	}
}