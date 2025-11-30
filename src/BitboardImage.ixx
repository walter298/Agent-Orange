export module Chess.BitboardImage;

import std;
export import Chess.Bitboard;

export namespace chess {
	struct RGB {
		std::uint8_t r = 0;
		std::uint8_t g = 0;
		std::uint8_t b = 0;
	};
	using ColorGetter = std::move_only_function<RGB(Bitboard)>;

	void drawBitboardImage(ColorGetter colorGetter, const std::string& filename);
}