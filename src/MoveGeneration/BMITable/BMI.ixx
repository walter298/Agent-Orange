module;

#include <immintrin.h>

export module Chess.LegalMoveGeneration:BMI;

export import Chess.Square;
export import Chess.Bitboard;
export import :ChainedMoveGenerator;

export namespace chess {
	Bitboard getMagicIndex(Bitboard pieces, Bitboard magicNum, std::uint8_t shift) {
		return (pieces * magicNum) >> shift;
	}

	template<typename Storage>
	struct BMI {
		Storage possiblePositions;
		Bitboard rays = 0;
		
		_forceinline Bitboard operator[](Bitboard allPieces) const {
			auto index = _pext_u64(allPieces, rays);
			return possiblePositions[index];
		}
	};
	using DynamicBMI = BMI<std::vector<Bitboard>>;
	using StaticBMI  = BMI<const Bitboard*>;

	template<typename Magic>
	struct MagicMaps {
		SquareMap<Magic> orthogonalMoveMap;
		SquareMap<Magic> diagonalMoveMap;
	};
}