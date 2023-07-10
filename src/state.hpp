#pragma once

#include "input.hpp"

constexpr double AVOIDANCE_RADIUS = 10.0;
constexpr double BLOCKING_RADIUS = 5.0;

struct RollbackInfo {
    int i;
    Pos pos;
    std::vector<double> volumes;
};

struct State {
    Input* const input;
    std::vector<Pos> placements;
    std::vector<double> volumes;
    long score;

    void init();
    long calc_score();
    void mountain();
    void SA();
    nlohmann::json to_json();
    void dump_scores();

    State(Input &input);

private:
    long delta_max, delta_min;
    double calc_i(size_t i, size_t k);
    void find_opt_volumes();
    RollbackInfo modify();
    void rollback(const RollbackInfo &info);
};
