#pragma once
#include "components.h"

namespace MapLabels {
    // Bounds and targets use the map's 940x880 coordinate space.
    struct Rect {
        int x, y, w, h;
    };

    struct Placement {
        Rect bounds;
        Point target;
        bool external;
    };

    std::vector<Placement> layout(const std::vector<ProvinceComponent>& provinces, const std::vector<Point>& textSizes);
}
