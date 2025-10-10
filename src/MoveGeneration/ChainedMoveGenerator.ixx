export module Chess.LegalMoveGeneration:ChainedMoveGenerator;

import std;

export import Chess.Bitboard;
export import :MoveGen;
import :PawnMoveGeneration;

export namespace chess {
	template<typename... MoveGenerators>
	class ChainedMoveGenerator {
	private:
		std::tuple<MoveGenerators...> m_generators;
	public:
		ChainedMoveGenerator(MoveGenerators... generators) :
			m_generators{ generators... }
		{
		}

		MoveGen operator()(Bitboard movingPieces, Bitboard empty) {
			return std::apply([&](auto... generators) {
				return (generators(movingPieces, empty) | ...);
			}, m_generators);
		}
	};

	template<typename M1, typename M2>
	ChainedMoveGenerator<M1, M2> operator|(M1 m1, M2 m2) {
		return { m1, m2 };
	}
}