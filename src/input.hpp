#pragma once

#include <bits/stdc++.h>
#include "nlohmann/json.hpp"

struct Pos {
    double x, y;
    Pos(double x, double y) : x(x), y(y) {}
    Pos() : x(0.0), y(0.0) {}

    double dot(const Pos &rhs) const;
    double dist(const Pos &rhs) const;  // 点 rhs までの距離
    double dist(const Pos &a, const Pos &b) const;  // 線分 ab までの距離
    
    bool operator<(const Pos &rhs) const;
    Pos &operator+=(const Pos &rhs);
    Pos &operator-=(const Pos &rhs);
    Pos &operator*=(const double &rhs);
    Pos &operator/=(const double &rhs);
    Pos operator+(const Pos &rhs) const;
    Pos operator-(const Pos &rhs) const;
    Pos operator*(const double &rhs) const;
    Pos operator/(const double &rhs) const;
};

struct Attendee {
    Pos pos;
    std::vector<double> tastes;
    Attendee (const nlohmann::json &j) {
        pos.x = j["x"].get<double>();
        pos.y = j["y"].get<double>();
        tastes = j["tastes"].get<std::vector<double>>();
    }
};

struct Pillar {
    Pos pos;
    double radius;
    Pillar (const nlohmann::json &j) {
        pos.x = j["center"][0].get<double>();
        pos.y = j["center"][1].get<double>();
        radius = j["radius"].get<double>();
    }
};

struct Input {
    double room_w, room_h, stage_w, stage_h;
    double stage_l, stage_u, stage_r, stage_d;
    std::vector<int> musicians;
    std::vector<Attendee> attendees;
    std::vector<Pillar> pillars;

    Input(const char *input_filename);
};
