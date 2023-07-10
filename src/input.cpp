#include "input.hpp"

double Pos::dot(const Pos &rhs) const {
    return x * rhs.x + y * rhs.y;
}
double Pos::dist(const Pos &rhs) const {
    return hypot(x - rhs.x, y - rhs.y);
}
// https://zenn.dev/boiledorange73/articles/0037-js-distance-pt-seg
double Pos::dist(const Pos &a, const Pos &b) const {
    const Pos ap = *this - a, ab = b - a;
    const Pos ba = a - b, bp = *this - b;
    if (ap.dot(ab) < 0) return INFINITY; //*/ this->dist(a);
    if (bp.dot(ba) < 0) return INFINITY; //*/ this->dist(b);
    return abs(ab.x * ap.y - ab.y * ap.x) / a.dist(b);
}

bool Pos::operator<(const Pos &rhs) const {
    return std::make_pair(x, y) < std::make_pair(rhs.x, rhs.y);
}
Pos &Pos::operator+=(const Pos &rhs) {
    x += rhs.x, y += rhs.y;
    return (*this);
}
Pos &Pos::operator-=(const Pos &rhs) {
    x -= rhs.x, y -= rhs.y;
    return (*this);
}
Pos &Pos::operator*=(const double &rhs) {
    x *= rhs, y *= rhs;
    return (*this);
}
Pos &Pos::operator/=(const double &rhs) {
    x /= rhs, y /= rhs;
    return (*this);
}
Pos Pos::operator+(const Pos &rhs) const {
    Pos result = *this;
    return (result += rhs);
}
Pos Pos::operator-(const Pos &rhs) const {
    Pos result = *this;
    return (result -= rhs);
}
Pos Pos::operator*(const double &rhs) const {
    Pos result = *this;
    return (result *= rhs);
}
Pos Pos::operator/(const double &rhs) const {
    Pos result = *this;
    return (result /= rhs);
}

Input::Input(const char *input_filename) {
    nlohmann::json input;
    std::ifstream reading(input_filename, std::ios::in);
    reading >> input;

    room_w = input["room_width"].get<double>();
    room_h = input["room_height"].get<double>();
    stage_w = input["stage_width"].get<double>();
    stage_h = input["stage_height"].get<double>();

    stage_l = input["stage_bottom_left"][0].get<double>();
    stage_u = input["stage_bottom_left"][1].get<double>();
    stage_r = stage_l + stage_w;
    stage_d = stage_u + stage_h;

    musicians = input["musicians"].get<std::vector<int>>();
    for (const auto &j : input["attendees"]) {
        attendees.push_back(j);
    }
    for (const auto &j : input["pillars"]) {
        pillars.push_back(j);
    }
}
