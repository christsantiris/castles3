#include "map_labels.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace MapLabels {
    static constexpr int WIDTH = 940;
    static constexpr int HEIGHT = 880;
    static constexpr int PAD = 4;

    class Mask {
        std::vector<int> pixels = std::vector<int>((WIDTH + 1) * (HEIGHT + 1));

    public:
        void add(const std::vector<Point>& polygon) {
            if (polygon.size() < 3) {
                return;
            }
            int minY = HEIGHT, maxY = 0;
            for (const auto& point : polygon) {
                minY = std::min(minY, point.y);
                maxY = std::max(maxY, point.y);
            }
            for (int y = std::max(0, minY); y < std::min(HEIGHT, maxY); y++) {
                std::vector<double> crossings;
                for (size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++) {
                    const auto& a = polygon[i];
                    const auto& b = polygon[j];
                    if ((a.y > y) != (b.y > y)) {
                        crossings.push_back(a.x + double(b.x - a.x) * (y - a.y) / (b.y - a.y));
                    }
                }
                std::sort(crossings.begin(), crossings.end());
                for (size_t i = 0; i + 1 < crossings.size(); i += 2) {
                    int left = std::max(0, (int)std::ceil(crossings[i]));
                    int right = std::min(WIDTH, (int)std::ceil(crossings[i + 1]));
                    for (int x = left; x < right; x++) {
                        pixels[(y + 1) * (WIDTH + 1) + x + 1] = 1;
                    }
                }
            }
        }

        void add(const ProvinceComponent& province) {
            add(province.polygon);
            for (const auto& island : province.islands) {
                add(island);
            }
        }

        void finish() {
            for (int y = 1; y <= HEIGHT; y++) {
                int row = 0;
                for (int x = 1; x <= WIDTH; x++) {
                    row += pixels[y * (WIDTH + 1) + x];
                    pixels[y * (WIDTH + 1) + x] = row + pixels[(y - 1) * (WIDTH + 1) + x];
                }
            }
        }

        int count(const Rect& r) const {
            if (r.x < 0 || r.y < 0 || r.x + r.w > WIDTH || r.y + r.h > HEIGHT) {
                return -1;
            }
            auto at = [&](int x, int y) { return pixels[y * (WIDTH + 1) + x]; };
            return at(r.x + r.w, r.y + r.h) - at(r.x, r.y + r.h) - at(r.x + r.w, r.y) + at(r.x, r.y);
        }

        bool fits(const Rect& r) const {
            return count(r) == r.w * r.h;
        }
    };

    static bool overlaps(const Rect& a, const Rect& b) {
        return a.x < b.x + b.w && b.x < a.x + a.w && a.y < b.y + b.h && b.y < a.y + a.h;
    }

    static Rect padded(const Rect& r) {
        return {r.x - PAD, r.y - PAD, r.w + PAD * 2, r.h + PAD * 2};
    }

    static double distance(Point a, Point b) {
        return double(a.x - b.x) * (a.x - b.x) + double(a.y - b.y) * (a.y - b.y);
    }

    static bool crosses(Point a, Point b, const Rect& r) {
        int steps = std::max(std::abs(a.x - b.x), std::abs(a.y - b.y));
        for (int step = 0; step <= steps; step++) {
            double t = steps == 0 ? 0 : double(step) / steps;
            double x = a.x + t * (b.x - a.x);
            double y = a.y + t * (b.y - a.y);
            if (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h) {
                return true;
            }
        }
        return false;
    }

    std::vector<Placement> layout(const std::vector<ProvinceComponent>& provinces, const std::vector<Point>& textSizes) {
        std::vector<Placement> result(provinces.size());
        std::vector<bool> placed(provinces.size(), false);
        std::vector<std::vector<Point>> targets(provinces.size());
        Mask land;
        for (const auto& province : provinces) {
            land.add(province);
        }
        land.finish();
        auto available = [&](const Rect& r) {
            for (size_t i = 0; i < result.size(); i++) {
                if (placed[i] && overlaps(padded(r), result[i].bounds)) {
                    return false;
                }
                if (placed[i] && result[i].external) {
                    const auto& b = result[i].bounds;
                    Point target = result[i].target;
                    Point start = {std::clamp(target.x, b.x - PAD, b.x + b.w + PAD),
                                   std::clamp(target.y, b.y - PAD, b.y + b.h + PAD)};
                    if (crosses(start, target, padded(r))) {
                        return false;
                    }
                }
            }
            return true;
        };

        // Reserve interior labels first. Querying the summed mask tests the
        // entire text rectangle, including bays between its corners.
        for (size_t i = 0; i < provinces.size(); i++) {
            Mask mask;
            mask.add(provinces[i]);
            mask.finish();
            Point preferred = provinces[i].hasLabelPosition ? provinces[i].labelPosition : Point{WIDTH / 2, HEIGHT / 2};
            double best = std::numeric_limits<double>::max();
            for (int y = PAD; y < HEIGHT - PAD; y += 2) {
                for (int x = PAD; x < WIDTH - PAD; x += 2) {
                    Rect r = {x - textSizes[i].x / 2, y - textSizes[i].y / 2, textSizes[i].x, textSizes[i].y};
                    double score = distance({x, y}, preferred);
                    if (score < best && mask.fits(padded(r)) && available(r)) {
                        result[i] = {r, {x, y}, false};
                        best = score;
                    }
                    if (x % 4 == 0 && y % 4 == 0 && mask.fits({x - 1, y - 1, 3, 3})) {
                        targets[i].push_back({x, y});
                    }
                }
            }
            if (targets[i].empty()) {
                for (int y = 0; y < HEIGHT && targets[i].empty(); y++) {
                    for (int x = 0; x < WIDTH; x++) {
                        if (mask.fits({x, y, 1, 1})) {
                            targets[i].push_back({x, y});
                            break;
                        }
                    }
                }
            }
            if (targets[i].empty()) {
                targets[i].push_back(preferred);
            }
            placed[i] = best < std::numeric_limits<double>::max();
        }

        for (size_t i = 0; i < provinces.size(); i++) {
            if (placed[i]) {
                continue;
            }
            Point preferred = provinces[i].hasLabelPosition ? provinces[i].labelPosition : targets[i].front();
            double best = std::numeric_limits<double>::max();
            Point chosenTarget = targets[i].front();
            for (int y = PAD; y < HEIGHT - PAD; y += 4) {
                for (int x = PAD; x < WIDTH - PAD; x += 4) {
                    Rect r = {x - textSizes[i].x / 2, y - textSizes[i].y / 2, textSizes[i].x, textSizes[i].y};
                    double score = distance({x, y}, preferred);
                    if (score >= best || land.count(padded(r)) != 0 || !available(r)) {
                        continue;
                    }
                    auto target = *std::min_element(targets[i].begin(), targets[i].end(), [&](Point a, Point b) {
                        return distance(a, {x, y}) < distance(b, {x, y});
                    });
                    bool obstructed = false;
                    Point start = {std::clamp(target.x, r.x - PAD, r.x + r.w + PAD),
                                   std::clamp(target.y, r.y - PAD, r.y + r.h + PAD)};
                    for (size_t other = 0; other < result.size(); other++) {
                        if (placed[other] && crosses(start, target, padded(result[other].bounds))) {
                            obstructed = true;
                            break;
                        }
                    }
                    if (obstructed) {
                        continue;
                    }
                    score += distance(target, {x, y}) * 2;
                    if (score < best) {
                        result[i] = {r, target, true};
                        chosenTarget = target;
                        best = score;
                    }
                }
            }
            if (best == std::numeric_limits<double>::max()) {
                // Fallback for a fully landlocked custom map: keep a callout
                // near the province rather than dropping its name entirely.
                result[i] = {{std::clamp(preferred.x - textSizes[i].x / 2, PAD, WIDTH - textSizes[i].x - PAD),
                              std::clamp(preferred.y - textSizes[i].y / 2, PAD, HEIGHT - textSizes[i].y - PAD),
                              textSizes[i].x, textSizes[i].y}, chosenTarget, true};
            }
            placed[i] = true;
        }
        return result;
    }
}
