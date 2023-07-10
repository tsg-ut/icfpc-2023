#include <bits/stdc++.h>
#include "nlohmann/json.hpp"
#include "input.hpp"
#include "state.hpp"

// 引数に入力ファイル名 (problem-*.json) をとる
int main(int argc, char **argv) {
    using namespace std;

    Input input(argv[1]);

    State state(input);
    cerr << "initial score: " << state.score << endl;
    // state.SA();
    state.mountain();
    cerr << "final score: " << state.score << endl;
    state.dump_scores();

    cout << state.to_json().dump(-1) << endl;
}
