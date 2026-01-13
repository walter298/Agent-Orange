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
	        Square::A8, Square::B8, Square::C8, Square::D8, Square::E8, Square::F8, Square::G8, Square::H8,
	        Square::A7, Square::B7, Square::C7, Square::D7, Square::E7, Square::F7, Square::G7, Square::H7,
	        Square::A6, Square::B6, Square::C6, Square::D6, Square::E6, Square::F6, Square::G6, Square::H6,
	        Square::A5, Square::B5, Square::C5, Square::D5, Square::E5, Square::F5, Square::G5, Square::H5,
	        Square::A4, Square::B4, Square::C4, Square::D4, Square::E4, Square::F4, Square::G4, Square::H4,
	        Square::A3, Square::B3, Square::C3, Square::D3, Square::E3, Square::F3, Square::G3, Square::H3,
	        Square::A2, Square::B2, Square::C2, Square::D2, Square::E2, Square::F2, Square::G2, Square::H2,
	        Square::A1, Square::B1, Square::C1, Square::D1, Square::E1, Square::F1, Square::G1, Square::H1
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
                pieces.castling.disallowKingsideCastling();
            }
            if (!castlingPrivileges.contains(queen)) {
                pieces.castling.disallowQueensideCastling();
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