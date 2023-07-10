#include "state.hpp"

constexpr long N_TRIAL = 200;
constexpr long TL = 360 * CLOCKS_PER_SEC;

namespace MMNMM{
    class splitmix64{
        unsigned long x;
    public:
        using result_type = unsigned long;
        constexpr unsigned long max() const {
            return std::numeric_limits<unsigned long>::max();
        }
        constexpr unsigned long min() const {
            return std::numeric_limits<unsigned long>::min();
        }
        constexpr explicit splitmix64(unsigned long seed = 0) : x(seed) {}
        constexpr unsigned long operator()() {
            unsigned long z = (x += 0x9e3779b97f4a7c15);
            z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
            z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
            return z ^ (z >> 31);
        }
    };
}
MMNMM::splitmix64 engine(std::random_device{}());

// 初期解の作成
void State::init() {
    // 出来るだけ疎なハニカムをつくったときの 1 辺の大きさ DISTANCE を求める
    const double EPS = 1e-7;
    double DISTANCE = 10, DISTANCE_NG = std::max(input->stage_h, input->stage_w);
    while (DISTANCE_NG - DISTANCE > EPS) {
        double M = (DISTANCE + DISTANCE_NG) / 2;
        double x = input->stage_l + AVOIDANCE_RADIUS;
        double y = input->stage_u + AVOIDANCE_RADIUS;
        int cnt = 0;
        for (size_t i = 0; i < input->musicians.size(); ++i) {
            x += M;
            if (x + AVOIDANCE_RADIUS > input->stage_r) {
                x = input->stage_l + AVOIDANCE_RADIUS + M * (++cnt % 2) / 2;
                y += M;
            }
        }
        if (y + AVOIDANCE_RADIUS <= input->stage_d) DISTANCE = M;
        else DISTANCE_NG = M;
    }

    // ミュージシャンの席を用意（誰が座るかは未定）
    std::vector<Pos> seats;
    double x = input->stage_l + AVOIDANCE_RADIUS;
    double y = input->stage_u + AVOIDANCE_RADIUS;
    int cnt = 0;
    for (size_t i = 0; i < input->musicians.size(); ++i) {
        assert(x - AVOIDANCE_RADIUS >= input->stage_l);
        assert(x + AVOIDANCE_RADIUS <= input->stage_r);
        assert(y - AVOIDANCE_RADIUS >= input->stage_u);
        assert(y + AVOIDANCE_RADIUS <= input->stage_d);
        seats.emplace_back(x, y);

        x += DISTANCE;
        if (x + AVOIDANCE_RADIUS > input->stage_r) {
            x = input->stage_l + AVOIDANCE_RADIUS + DISTANCE * (++cnt % 2) / 2;
            y += DISTANCE;
        }
    }

    // 他の人が座った場合のうれしさと比較した時の差が最大になるような(ミュージシャン,席)の組み合わせから埋めていく
    size_t n_musicians = input->musicians.size();
    std::map<int, std::vector<size_t>> instr_remains;
    for (size_t i = 0; i < n_musicians; ++i) {
        instr_remains[input->musicians.at(i)].push_back(i);
    }
    std::vector<bool> is_seat_used(n_musicians);
    placements = std::vector<Pos>(n_musicians);
    for (size_t t = 0; t < n_musicians; ++t) {
        // 最もうれしさの差が大きい席を探す
        std::vector<double> diff(n_musicians, -INFINITY);
        std::vector<int> best_instr(n_musicians, -1);
        for (size_t seat_i = 0; seat_i < n_musicians; ++seat_i) {
            if (is_seat_used.at(seat_i)) continue;

            double best_sum = -1e18, second_sum = -1e18;
            for (const auto &[instr, v] : instr_remains) {
                int k = v.front();
                placements.at(k) = seats.at(seat_i);
                double sum = 0.0;
                for (size_t i = 0; i < input->attendees.size(); ++i) {
                    sum += calc_i(i, k);
                }
                if (best_sum < sum) {
                    second_sum = best_sum;
                    best_sum = sum;
                    best_instr.at(seat_i) = instr;
                } else if (second_sum < sum) {
                    second_sum = sum;
                }
            }
            diff.at(seat_i) = best_sum - second_sum;
        }
        int chosen_seat = std::distance(diff.begin(), std::max_element(diff.begin(), diff.end()));
        int instr = best_instr[chosen_seat], k = instr_remains[instr].back();

        placements.at(k) = seats.at(chosen_seat);
        // std::cerr << "t:" << t
        //     << " diff:" << diff.at(chosen_seat)
        //     << " chosen seat:" << chosen_seat
        //     << " instr:" << instr
        //     << " k:" << k << std::endl;
        is_seat_used.at(chosen_seat) = true;
        instr_remains[instr].pop_back();
        if (instr_remains[instr].empty()) instr_remains.erase(instr);
    }

    volumes.resize(n_musicians);
    find_opt_volumes();
    score = calc_score();
}

void State::find_opt_volumes() {
    size_t n_musicians = input->musicians.size();

    for (size_t k = 0; k < n_musicians; ++k) {
        double sum = 0.0;
        for (size_t i = 0; i < input->attendees.size(); ++i) {
            sum += calc_i(i, k);
        }
        if (sum >= 0) volumes[k] = 10.0;
        else volumes[k] = 0.0;
    }
}

// スコア計算
long State::calc_score() {
    size_t n_musicians = input->musicians.size();
    // q[k] のテーブルを作成
    std::vector<double> q(n_musicians, 1.0);
    for (size_t i = 0; i < n_musicians; ++i) {
        for (size_t j = i + 1; j < n_musicians; ++j) {
            if (input->musicians.at(i) != input->musicians.at(j)) continue;
            double d_inv = 1 / placements.at(i).dist(placements.at(j));
            q.at(i) += d_inv;
            q.at(j) += d_inv;
        }
        // 先に volume をかけてしまう
        q.at(i) *= volumes.at(i);
    }

    long score = 0;
    for (size_t k = 0; k < n_musicians; ++k) {
        for (size_t i = 0; i < input->attendees.size(); ++i) {
            score += ceil(q.at(k) * calc_i(i, k));
        }
    }
    return score;
}

// 山を登るついでにスコア増減のデータも取る
void State::mountain() {
    int cnt = 0;
    long prev_score = score;
    clock_t deadline = clock() + TL;
    // for (int t = 0; t < N_TRIAL; ++t) {
    while (clock() < deadline) {
        RollbackInfo info = modify();
        long new_score = calc_score(), delta = abs(new_score - prev_score);
        std::cerr << "trial " << ++cnt << ": " << new_score << std::endl;
        if (delta_min > delta) delta_min = delta;
        if (delta_max < delta) delta_max = delta;
        if (prev_score < new_score) {
            prev_score = new_score;
        } else {
            rollback(info);
        }
    }
    score = prev_score;
}

void State::SA() {
    // スコア変動の統計を取る
    mountain();

    int cnt = N_TRIAL;
    long prev_score = score, best_score = score;
    std::vector<Pos> best_placements = placements;
    std::vector<double> best_volumes = volumes;

    double start_temp = delta_max * 0.25 + delta_min * 0.25;
    double end_temp = delta_min * 0.5;
    std::uniform_real_distribution<> distr(0.0, 1.0);

    clock_t start_time = clock(), prev_time;
    clock_t deadline = start_time + TL;
    while ((prev_time = clock()) < deadline) {
        RollbackInfo info = modify();
        long new_score = calc_score();
        std::cerr << "trial " << ++cnt << ": " << new_score << std::endl;

        double temp = start_temp + (end_temp - start_temp) * (prev_time - start_time) / TL;
        double prob = exp((new_score - prev_score) / temp);
        if (prob > distr(engine)) {
            prev_score = new_score;
        } else {
            rollback(info);
        }

        if (best_score < new_score) {
            best_score = new_score;
            best_placements = placements;
            best_volumes = volumes;
        }
    }
    score = best_score;
    placements = best_placements;
    volumes = best_volumes;
}

nlohmann::json State::to_json() {
    nlohmann::json output;
    output["placements"] = nlohmann::json::array();
    for (const auto &pos : placements) {
        output["placements"].push_back({{"x", pos.x}, {"y", pos.y}});
    }
    output["volumes"] = volumes;
    return output;
}

void State::dump_scores() {
    long sum = 0;

    ////////// ここから //////////
    size_t n_musicians = input->musicians.size();
    // q[k] のテーブルを作成
    std::vector<double> q(n_musicians, 1.0);
    for (size_t i = 0; i < n_musicians; ++i) {
        for (size_t j = i + 1; j < n_musicians; ++j) {
            if (input->musicians.at(i) != input->musicians.at(j)) continue;
            double d_inv = 1 / placements.at(i).dist(placements.at(j));
            q.at(i) += d_inv;
            q.at(j) += d_inv;
        }
        // 先に volume をかけてしまう
        q.at(i) *= volumes.at(i);
    }

    for (size_t k = 0; k < n_musicians; ++k) {
        for (size_t i = 0; i < input->attendees.size(); ++i) {
    ////////// ここまでスコア関数のコピペ //////////
            sum += ceil(q.at(k) * calc_i(i, k));
        }
        std::cerr << "score " << k << ": " << sum << std::endl;
        sum = 0;
    }
}

State::State(Input &input) : input(&input), score(0), delta_max(0), delta_min(INT64_MAX) {
    init();
}

// I_i(k)
double State::calc_i(size_t i, size_t k) {
    const Pos &musician_pos = placements.at(k);
    const Pos &attendee_pos = input->attendees.at(i).pos;

    // 他のミュージシャンが間に入っていないか？
    for (size_t j = 0; j < input->musicians.size(); ++j) {
        if (k == j) continue;
        if (placements.at(j).dist(musician_pos, attendee_pos) < BLOCKING_RADIUS) {
            return 0.0;
        }
    }

    // 柱に遮られていないか？
    for (const auto &pillar : input->pillars) {
        if (pillar.pos.dist(musician_pos, attendee_pos) < pillar.radius) {
            return 0.0;
        }
    }

    const Pos ik = attendee_pos - musician_pos;
    double d_sq = ik.dot(ik);
    return ceil(1e6 * input->attendees.at(i).tastes.at(input->musicians.at(k)) / d_sq);
}

RollbackInfo State::modify() {
    RollbackInfo info;
    std::uniform_real_distribution<> distr_x(
        input->stage_l + AVOIDANCE_RADIUS, input->stage_r - AVOIDANCE_RADIUS);
    std::uniform_real_distribution<> distr_y(
        input->stage_u + AVOIDANCE_RADIUS, input->stage_d - AVOIDANCE_RADIUS);
    
    info.i = engine() % input->musicians.size();
    info.pos = placements.at(info.i);
    info.volumes = volumes;
    do {
        placements.at(info.i).x = distr_x(engine);
        placements.at(info.i).y = distr_y(engine);
    } while (
        std::any_of(placements.begin(), placements.end(), [&](const Pos &p) {
            double d = placements.at(info.i).dist(p);
            if (d == 0) return false;
            return d < AVOIDANCE_RADIUS;
        })
    );
    find_opt_volumes();
    return info;
}

void State::rollback(const RollbackInfo &info) {
    placements.at(info.i) = info.pos;
    volumes = info.volumes;
}
