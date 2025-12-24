export module Chess.Position:PositionHasher;

export import :PositionObject;

export namespace chess {
	struct PositionHasher {
		size_t operator()(const Position& pos) const {
			auto [white, black] = pos.getColorSides();

			auto allPieces = white.calcAllLocations() | black.calcAllLocations();

			return static_cast<size_t>(allPieces);
		}
	};
}