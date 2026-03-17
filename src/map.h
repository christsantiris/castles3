#pragma once
#include <vector>
#include <string>
#include <SDL2/SDL.h>

struct Province {
    int id;
    std::string name;
    int owner;
    std::vector<SDL_Point> polygon;
    bool isSelected = false;

    bool containsPoint(int x, int y) const;
};

class Map {
public:
    std::vector<Province> provinces;

    void load(const std::string& path);
    void handleClick(int x, int y);
    void render(SDL_Renderer* renderer);
};