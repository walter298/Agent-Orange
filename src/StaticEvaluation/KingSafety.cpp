module Chess.Evaluation:KingSafety;

import std;
import Chess.SquareZone;
import :Constants;

namespace chess {
	class DistanceTable {
	private:
		std::uint8_t m_table[64][64];
	public:
		DistanceTable() {
			using std::views::cartesian_product;
			for (auto [a, b] : cartesian_product(SQUARE_ARRAY, SQUARE_ARRAY)) {
				auto [aFile, aRank] = fileRankOf(a);
				auto [bFile, bRank] = fileRankOf(b);
				auto dx = std::abs(aFile - bFile);
				auto dy = std::abs(aRank - bRank);
				m_table[a][b] = static_cast<std::uint8_t>(std::max(dx, dy));
			}
		}

		constexpr std::uint8_t operator()(Square a, Square b) const {
			return m_table[a][b];
		}
	};

	DistanceTable distanceTable;

	Rating calcEnemyProximityPenaltyImpl(Square allyKingPos, Bitboard enemySquares, Rating penalty) {
		auto ret = 0_rt;

		auto enemySquare = Square::None;
		while (nextSquare(enemySquares, enemySquare)) {
			auto distFromKing = distanceTable(allyKingPos, enemySquare);
			ret += penalty * static_cast<Rating>(distFromKing);
		}

		return ret;
	}

	Rating calcEnemyProximityPenalty(Square allyKingPos, const PieceState& enemyPieces, Bitboard enemyDestSquares) {
		auto ret = 0_rt;

		//penalize enemy pieces being close to the king
		auto pieceTypes = ALL_PIECE_TYPES | std::views::drop(1); //exclude king
		for (auto pieceType : pieceTypes) {
			auto enemyPieceLocations = enemyPieces[pieceType];
			auto penalty = pieceRatings[pieceType] * PIECE_PROXIMITY_FACTOR;
			ret += calcEnemyProximityPenaltyImpl(allyKingPos, enemyPieceLocations, penalty);
		}

		//penalize enemy destination squares close to the king
		ret += calcEnemyProximityPenaltyImpl(allyKingPos, enemyDestSquares, DESTINATION_SQUARE_PROXIMITY_FACTOR);

		return ret;
	}

	Rating calcKingSafetyRating(const Position& pos, const PositionData& posData) {
		auto ret = 0_rt;
		auto [white, black] = pos.getColorSides();

		auto whiteKingPos = nextSquare(white[King]);
		ret -= calcEnemyProximityPenalty(whiteKingPos, black, posData.blackSquares);

		auto blackKingPos = nextSquare(black[King]);
		ret += calcEnemyProximityPenalty(blackKingPos, white, posData.whiteSquares);

		return ret;
	}
}