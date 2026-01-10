export module Chess.MoveGeneration:ChainedMoveGenerator;

import std;

export import Chess.Bitboard;
export import Chess.MoveGen;
import :PawnMoveGeneration;

export namespace chess {
	template<typename... MoveGenerators>
	class ChainedMoveGenerator {
	private:
		std::tuple<MoveGenerators...> m_generators;
	public:
		constexpr ChainedMoveGenerator(MoveGenerators... generators) :
			m_generators{ generators... }
		{
		}

		MoveGen operator()(Bitboard movingPieces, Bitboard empty) const {
			return std::apply([&](auto... generators) {
				return (generators(movingPieces, empty) | ...);
			}, m_generators);
		}
	};

	template<typename M1, typename M2>
	consteval ChainedMoveGenerator<M1, M2> operator|(M1 m1, M2 m2) {
		return { m1, m2 };
	}
}