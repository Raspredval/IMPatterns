#pragma once
static_assert(__cplusplus >= 202302L, "requires C++23 minimum version");

#include <span>
#include <vector>
#include "Match.hpp"
#include "UserData.hpp"

namespace imp {
    struct Capture {
        imp::Match      m;
        imp::UserData   d;
    };

    using Captures =
        std::vector<Capture>;
    using CapturesList =
        std::vector<Captures>;
    using CapturesView =
        std::span<const Capture>;
}