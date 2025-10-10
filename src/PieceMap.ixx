export module Chess.PieceMap;

import std;
export import Chess.PieceType;

namespace chess {
	export template<typename T>
	class PieceMap {
	protected:
		std::array<T, 6> m_data;
	public:
		constexpr PieceMap() {
			std::ranges::fill(m_data, T{});
		}
		template<size_t N>
		constexpr PieceMap(const std::pair<Piece, T> (&values)[N]) {
			for (const auto& [piece, t] : values) {
				m_data[piece] = t;
			}
		}
		constexpr decltype(auto) operator[](this auto&& self, Piece piece) {
			return self.m_data[piece];
		}
		auto begin(this auto&& self) {
			return self.m_data.begin();
		}
		auto end(this auto&& self) {
			return self.m_data.end();
		}
	};
}