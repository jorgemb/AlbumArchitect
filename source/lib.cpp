#include <format>

#include "lib.hpp"

library::library()
    : name {std::format("{}", "AlbumArchitect")} {}
