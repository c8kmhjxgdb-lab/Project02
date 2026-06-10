#pragma once

#include <cstdlib>
#include <iostream>

namespace TestSupport {

inline void require(bool condition, const char* message) {
    if (condition) return;

    std::cerr << "Test failed: " << message << '\n';
    std::exit(1);
}

}  // namespace TestSupport
