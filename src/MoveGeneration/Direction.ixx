export module Chess.LegalMoveGeneration:Direction;

import std;
import Chess.Bitboard;

//    noWe  north noEa  -left shifts
//     + 7 + 8 + 9
//        \  |  /
// west - 1 < -0 -> + 1 east
//          /  |  \
//       - 9 - 8 - 7 
//   soWe south soEa -right shifts

export namespace chess {
	namespace dir {
		constexpr Bitboard NOT_A_FILE = 0xfefefefefefefefe;
		constexpr Bitboard NOT_H_FILE = 0x7f7f7f7f7f7f7f7f;
		constexpr Bitboard NOT_AB_FILE = 0xfcfcfcfcfcfcfcfc;
		constexpr Bitboard NOT_GH_FILE = 0x3f3f3f3f3f3f3f3f;
		
		template<typename T>
		concept Direction = requires(T) {
			T::move(Bitboard{});
			{ T::NON_BORDERS } -> std::same_as<const Bitboard&>;
		};

		namespace sliding {
			struct West {
				static constexpr Bitboard NON_BORDERS = NOT_H_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 1;
				}
			};
			struct East {
				static constexpr Bitboard NON_BORDERS = NOT_A_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return (bitboard << 1);
				}
			};
			struct NorthWest {
				static constexpr Bitboard NON_BORDERS = NOT_H_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard << 7;
				}
			};
			struct North {
				static constexpr Bitboard NON_BORDERS = std::numeric_limits<Bitboard>::max();
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard << 8;
				}
			};
			struct NorthEast {
				static constexpr Bitboard NON_BORDERS = NOT_A_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard << 9;
				}
			};
			struct SouthEast {
				static constexpr Bitboard NON_BORDERS = NOT_A_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 7;
				}
			};
			struct South {
				static constexpr Bitboard NON_BORDERS = std::numeric_limits<Bitboard>::max();
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 8;
				}
			};
			struct SouthWest {
				static constexpr Bitboard NON_BORDERS = NOT_H_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 9;
				}
			};
		}

		//knight directions
		/*			noNoWe    noNoEa //left shifts
					   + 15 + 17
						 |     |
			noWeWe + 6 __|     |__ + 10  noEaEa
						   \ /
						  > 0 <
						__ / \ __
		   soWeWe - 10   |     |   -6  soEaEa
						 |     |
						-17 - 15
					soSoWe    soSoEa*/ //right shifts
		namespace knight {
			struct NorthNorthEast {
				static constexpr Bitboard NON_BORDERS = NOT_A_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard << 17;
				}
			};
			struct NorthEastEast {
				static constexpr Bitboard NON_BORDERS = NOT_AB_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard << 10;
				}
			};
			struct SouthEastEast {
				static constexpr Bitboard NON_BORDERS = NOT_AB_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 6;
				}
			};
			struct SouthSouthEast {
				static constexpr Bitboard NON_BORDERS = NOT_A_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 15;
				}
			};
			struct NorthNorthWest {
				static constexpr Bitboard NON_BORDERS = NOT_H_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard << 15;
				}
			};
			struct NorthWestWest {
				static constexpr Bitboard NON_BORDERS = NOT_GH_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard << 6;
				}
			};
			struct SouthWestWest {
				static constexpr Bitboard NON_BORDERS = NOT_GH_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 10;
				}
			};
			struct SouthSouthWest {
				static constexpr Bitboard NON_BORDERS = NOT_H_FILE;
				static constexpr Bitboard move(Bitboard bitboard) {
					return bitboard >> 17;
				}
			};
		}

	}
}