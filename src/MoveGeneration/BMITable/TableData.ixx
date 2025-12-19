export module Chess.LegalMoveGeneration:TableData;

import std;
import Chess.EnvironmentVariable;

export namespace chess {
	std::filesystem::path getTablePath() {
		return getAssetDirectoryPath() / "BMI_Table.json";
	}
	constexpr const char* ORTHOGONAL_MOVE_MAP_KEY  = "orthogonal_move_map";
	constexpr const char* DIAGONAL_MOVE_MAP_KEY    = "diagonal_move_map";
	constexpr const char* TOTAL_POSITION_COUNT_KEY = "position_count";
	constexpr const char* RAYS_KEY                 = "rays";
	constexpr const char* POSITIONS_KEY            = "positions";
}