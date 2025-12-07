module;

#include <SDL3/SDL.h>

module Chess.BitboardImage;

import std;

import Chess.Square;
import Chess.EnvironmentVariable;

namespace chess {
	struct RenderData {
		constexpr static float SQUARE_LEN = 50;

		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;

		RenderData() {
			SDL_Init(SDL_INIT_VIDEO);

			window = SDL_CreateWindow("Bitboard Image", static_cast<int>(SQUARE_LEN) * 8, static_cast<int>(SQUARE_LEN) * 8, 0);
			if (!window) {
				std::println("Error creating window: {}", SDL_GetError());
				std::exit(-1);
			}

			renderer = SDL_CreateRenderer(window, nullptr);
			if (!renderer) {
				std::println("Error creating renderer: {}", SDL_GetError());
				std::exit(-1);
			}
		}

		~RenderData() {
			if (renderer) {
				SDL_DestroyRenderer(renderer);
			}
			if (window) {
				SDL_DestroyWindow(window);
			}
			SDL_Quit();
		}
	};

	void renderBitboard(SDL_Renderer* renderer, ColorGetter& colorGetter) {
		constexpr auto SQUARE_COORDS = SQUARE_ARRAY | std::views::transform([](Square square) {
			return std::pair{ fileOf(square), rankOf(square) };
		});

		auto makeCell = [](int file, int rank) -> SDL_FRect {
			constexpr auto BOTTOM_LEVEL = (7 * RenderData::SQUARE_LEN);
			return SDL_FRect{
				file * RenderData::SQUARE_LEN, BOTTOM_LEVEL - (rank * RenderData::SQUARE_LEN),
				RenderData::SQUARE_LEN,  RenderData::SQUARE_LEN
			};
		};

	
		for (auto [file, rank] : SQUARE_COORDS) {
			auto cell = makeCell(file, rank);

			Bitboard bit = 1ull << (rank * 8 + file);
			auto [r, g, b] = colorGetter(bit);

			SDL_SetRenderDrawColor(renderer, r, g, b, 255);
			SDL_RenderFillRect(renderer, &cell);
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			SDL_RenderRect(renderer, &cell);
		}
	}

	void drawBitboardImage(ColorGetter colorGetter, const std::string& filename)
	{
		RenderData renderData;
		SDL_RenderClear(renderData.renderer);

		renderBitboard(renderData.renderer, colorGetter);

		SDL_Rect rect{ 0, 0,
			static_cast<int>(RenderData::SQUARE_LEN) * 8,
			static_cast<int>(RenderData::SQUARE_LEN) * 8
		};
		auto surface = SDL_RenderReadPixels(renderData.renderer, &rect);
		if (!surface) {
			std::println("Error reading pixels: {}", SDL_GetError());
			return;
		}
		auto path = getBitboardImageDirectoryPath() / filename;
		auto pathStr = path.string();
		if (!SDL_SaveBMP(surface, pathStr.c_str())) {
			std::println("Error saving image: {}", SDL_GetError());
		}
		SDL_DestroySurface(surface);
	}
}