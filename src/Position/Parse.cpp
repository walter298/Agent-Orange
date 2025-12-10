module Chess.Position:Parse;

namespace chess {
    Piece parsePiece(char chr) {
        auto piece = Piece::None;
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
            std::exit(-1);
        }
        return piece;
    }

    void parseRow(std::string_view rowStr, std::span<const Square> squareRow, PieceState& whitePieces, PieceState& blackPieces) {
        size_t squareIndex = 0;

    	for (auto squareChr : rowStr) {
            if (std::isdigit(squareChr)) { //skip empty squares
                squareIndex += squareChr - '0';
                continue;
            }
            auto piece = parsePiece(static_cast<char>(std::tolower(static_cast<unsigned char>(squareChr))));
            if (std::islower(static_cast<unsigned char>(squareChr))) {
                addSquare(blackPieces[piece], squareRow[squareIndex]);
            } else {
                addSquare(whitePieces[piece], squareRow[squareIndex]);
            }
            squareIndex++;
        }
    }

    void parseBoard(std::string_view board, PieceState& white, PieceState& black) {
        auto strRows = board | std::views::split('/') | std::views::transform([](auto&& rng) {
            return std::string_view( rng.data(), rng.size());
        });

        constexpr std::array SQUARES_FEN_ORDER = {
            A8, B8, C8, D8, E8, F8, G8, H8,
            A7, B7, C7, D7, E7, F7, G7, H7,
            A6, B6, C6, D6, E6, F6, G6, H6,
            A5, B5, C5, D5, E5, F5, G5, H5,
            A4, B4, C4, D4, E4, F4, G4, H4,
            A3, B3, C3, D3, E3, F3, G3, H3,
            A2, B2, C2, D2, E2, F2, G2, H2,
            A1, B1, C1, D1, E1, F1, G1, H1
        };

        auto squareRows = SQUARES_FEN_ORDER | std::views::chunk(8) | std::views::transform([](auto&& rng) {
			return std::span{ rng.data(), rng.size() };
        });
        
        for (const auto& [rowStr, squareRow] : std::views::zip(strRows, squareRows)) {
            parseRow(rowStr, squareRow, white, black);
        }
    }

	void parseCastlingPrivileges(std::string_view castlingPrivileges, PieceState& white, PieceState& black) {
        auto verify = [&](PieceState& pieces, char king, char queen) {
            if (!castlingPrivileges.contains(king)) {
                pieces.disallowKingsideCastling();
            }
            if (!castlingPrivileges.contains(queen)) {
                pieces.disallowQueensideCastling();
            }
        };
        verify(white, 'K', 'Q');
        verify(black, 'k', 'q');
	}

    void parseEnPessantSquare(std::string_view enPessantSquareStr, bool isWhite, PieceState& enemies) {
        auto enPessantSquare = parseSquare(enPessantSquareStr);
        if (enPessantSquare) {
            auto jumpedPawn = isWhite ? southSquare(*enPessantSquare) : northSquare(*enPessantSquare);
            enemies.doubleJumpedPawn = jumpedPawn;
        } 
    }
}