#ifndef COMMON_H
#define COMMON_H

#include <filesystem>

namespace fs = std::filesystem;
using namespace std::literals; // NOLINT(*-global-names-in-headers)

// NOLINTNEXTLINE(cert-err58-cpp)
static inline auto const resources_dir = fs::path(TEST_RESOURCES_DIR);

#endif  // COMMON_H
