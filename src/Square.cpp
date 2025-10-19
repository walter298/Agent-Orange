module;

#include <magic_enum/magic_enum.hpp>

module Chess.Square;

namespace chess {
	std::optional<Square> parseSquare(std::string_view squareStr) {
		if (squareStr.size() != 2) {
			return std::nullopt;
		}
		char buff[2] = { static_cast<char>(std::toupper(squareStr[0])), squareStr[1] };
		return magic_enum::enum_cast<Square>(std::string_view{ buff, 2 });
	}
}