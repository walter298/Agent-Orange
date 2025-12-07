export module Chess.BitboardImage;

import std;

export import Chess.Bitboard;
export import Chess.Position;

export namespace chess {
	struct RGB {
		std::uint8_t r = 0;
		std::uint8_t g = 0;
		std::uint8_t b = 0;
	};
	constexpr RGB RED       = { 255, 0, 0 };
	constexpr RGB GREEN     = { 0, 255, 0 };
	constexpr RGB BLUE      = { 0, 0, 255 };
	constexpr RGB PURPLE    = { 97, 10, 255 };
	constexpr RGB YELLOW    = { 255, 255, 0 };
	constexpr RGB BROWN     = { 150, 75, 0 };
	constexpr RGB LIGHT_TAN = { 222, 184, 135 };
	constexpr RGB WHITE     = { 255, 255, 255 };

	using ColorGetter = std::move_only_function<RGB(Bitboard)>;

	constexpr auto getDefaultColorGetter(Bitboard board, RGB rgb = GREEN) {
		return [=](Bitboard bit) -> RGB {
			if (board & bit) {
				return rgb;
			} else {
				return YELLOW;
			}
		};
	}

	void drawPieceLocations(const Position& pos);
	void drawBitboardImage(ColorGetter colorGetter, const std::string& filename);
}