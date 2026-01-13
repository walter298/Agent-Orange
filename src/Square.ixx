export module Chess.Square;

import std;
import nlohmann.json;

import Chess.Assert;
import Chess.Bitboard;
import Chess.RankCalculator;

export namespace chess {
    enum class Square : std::uint8_t {
        A1, B1, C1, D1, E1, F1, G1, H1,
        A2, B2, C2, D2, E2, F2, G2, H2,
        A3, B3, C3, D3, E3, F3, G3, H3,
        A4, B4, C4, D4, E4, F4, G4, H4,
        A5, B5, C5, D5, E5, F5, G5, H5,
        A6, B6, C6, D6, E6, F6, G6, H6,
        A7, B7, C7, D7, E7, F7, G7, H7,
        A8, B8, C8, D8, E8, F8, G8, H8,
        None
    };

    constexpr Square SQUARE_ARRAY[64] = {
        Square::A1, Square::B1, Square::C1, Square::D1, Square::E1, Square::F1, Square::G1, Square::H1,
        Square::A2, Square::B2, Square::C2, Square::D2, Square::E2, Square::F2, Square::G2, Square::H2,
        Square::A3, Square::B3, Square::C3, Square::D3, Square::E3, Square::F3, Square::G3, Square::H3,
        Square::A4, Square::B4, Square::C4, Square::D4, Square::E4, Square::F4, Square::G4, Square::H4,
        Square::A5, Square::B5, Square::C5, Square::D5, Square::E5, Square::F5, Square::G5, Square::H5,
        Square::A6, Square::B6, Square::C6, Square::D6, Square::E6, Square::F6, Square::G6, Square::H6,
        Square::A7, Square::B7, Square::C7, Square::D7, Square::E7, Square::F7, Square::G7, Square::H7,
        Square::A8, Square::B8, Square::C8, Square::D8, Square::E8, Square::F8, Square::G8, Square::H8
	};
    constexpr int rankOf(Square square) {
        return static_cast<int>(square) / 8;
    }
    constexpr int fileOf(Square square) {
        return static_cast<int>(square) % 8;
    }
    struct FileRank {
        int file = 0;
        int rank = 0;
    };
    constexpr FileRank fileRankOf(Square square) {
        return { fileOf(square), rankOf(square) };
    }
    constexpr Bitboard leftFile(Square square) {
        auto fileIndex = fileOf(square);
        return fileIndex == 0 ? 0 : calcFile(fileIndex - 1);
    }
    constexpr Bitboard rightFile(Square square) {
        auto fileIndex = fileOf(square);
        return fileIndex == 7 ? 0 : calcFile(fileIndex + 1);
    }
    constexpr Square westSquare(Square square) {
        if (fileOf(square) == 0) {
            return Square::None;
        }
        return static_cast<Square>(static_cast<int>(square) - 1);
    }
    constexpr Square eastSquare(Square square) {
        if (fileOf(square) == 7) {
            return Square::None;
        }
        return static_cast<Square>(static_cast<int>(square) + 1);
    }
    constexpr Square southSquare(Square square) {
        auto num = static_cast<std::underlying_type_t<Square>>(square);
        if (num < 8) { //if square is in [A1, H1], there are no more south squares
            return Square::None;
        }
        return static_cast<Square>(num - 8);
    }
    constexpr Square northSquare(Square square) {
        auto num = static_cast<std::underlying_type_t<Square>>(square);
        if (num > 55) {
            return Square::None;
        }
        return static_cast<Square>(num + 8);
    }
    constexpr bool onEdgeOfBoard(Square square) {
        return rankOf(square) == 0 || rankOf(square) == 7 || fileOf(square) == 0 || fileOf(square) == 7;
    }

    constexpr Bitboard movePiece(Bitboard board, Square from, Square to) {
        board &= ~(Bitboard{ 1 } << static_cast<int>(from));
        board |= (Bitboard{ 1 } << static_cast<int>(to));
        return board;
    }

    constexpr Square nextSquare(Bitboard board) {
        return static_cast<Square>(std::countr_zero(board));
    }

    template<std::same_as<Square>... Squares>
    constexpr Bitboard makeBitboard(Squares... squares) requires(sizeof...(Squares) > 0) {
        Bitboard ret = 0;
        ((ret |= Bitboard{ 1 } << static_cast<int>(squares)), ...);
        return ret;
    }

    //for iterating over each square
    constexpr bool nextSquare(Bitboard& board, Square& square) {
        if (!board) {
            square = Square::None;
            return false;
        }
        square = nextSquare(board);
        board &= (board - 1);
        return true;
    }

    constexpr Square getNthSetSquare(Bitboard board, int n) {
        auto currSquare = Square::None;
        for (int i = 0; i <= n; i++) {
            nextSquare(board, currSquare);
        }
        return currSquare;
    }

    template<std::same_as<Square>... Squares>
	constexpr bool containsSquare(Bitboard bitboard, Squares... squares) requires(sizeof...(Squares) > 0) {
        return (((bitboard & makeBitboard(squares)) != 0) && ...);
    }

    constexpr void removeSquare(Bitboard& bitboard, Square square) {
        bitboard &= ~makeBitboard(square);
    }

    constexpr void addSquare(Bitboard& bitboard, Square square) {
        bitboard |= makeBitboard(square);
    }

    constexpr void moveSquare(Bitboard& bitboard, Square from, Square to) {
        removeSquare(bitboard, from);
        addSquare(bitboard, to);
    }

    std::optional<Square> parseSquare(std::string_view square);

    template<typename T>
    class SquareMap {
    public:
        using Buffer = std::array<T, 64>;
    private:
        Buffer m_entries;
    public:
        constexpr SquareMap() {
            std::ranges::fill(m_entries, T{});
        }
        constexpr explicit SquareMap(const Buffer& buffer) : m_entries{ buffer } {}
        decltype(auto) operator[](this auto&& self, Square square) {
            return std::forward_like<decltype(self)>(self.m_entries[static_cast<size_t>(square)]);
        }
        auto& get(this auto&& self) {
            return std::forward_like<decltype(self)>(self.m_entries);
        }
    };

    template<typename T>
    void from_json(const nlohmann::json& j, SquareMap<T>& map) {
        map = SquareMap{ j.get<typename SquareMap<T>::Buffer>() };
    }
    template<typename T>
    void to_json(nlohmann::json& j, const SquareMap<T>& map) {
        j = map.get();
    }
}