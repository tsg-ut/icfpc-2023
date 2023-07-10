# ICFP Programming Contest 2023 v3

- https://www.icfpcontest.com/specification_v3.pdf を読んでいくぞの気持ち
- 各 musician を stage に配置し、会場にいる attendee 全員の幸福度を最大にする。

## 問題の仕様

### 入力

- 部屋の形
  - サイズ
    - room_height: float
    - room_width: float
  - stage の寸法と位置（部屋の中にステージがある）
    - stage_height: float
    - stage_width: float
    - stage_bottom_left: [float, float]
      - x 座標と y 座標
- musician の配置
  - musician のリストと彼らが演奏している instrument
    - musicians: int list
      - 各 musician が使う instrument
- attendee の配置
  - 部屋の中での位置、各 instrument の好み
    - attendees: [{'x': float, 'y': float, 'tastes': float list}]
      - attendee の位置、各楽器に対する好み
- 障害物（ルール拡張）
  - 音を遮る柱
    - pillars: [{'center': [float, float], 'radius': float}]

```json
{
    "room_width": 2000.0,
    "room_height": 5000.0,
    "stage_width": 1000.0,
    "stage_height": 200.0,
    "stage_bottom_left": [500.0, 0.0],
    "musicians": [0, 1, 0],
    "attendees": [
        { "x": 100.0, "y": 500.0, "tastes": [1000.0, -1000.0] },
        { "x": 200.0, "y": 1000.0, "tastes": [200.0, 200.0] },
        { "x": 1100.0, "y": 800.0, "tastes": [800.0, 1500.0] }
    ],
    "pillars": [
        {"center": [500.0, 1000.0], "radius": 5.0},
        ...
    ]
}
```

### 出力

- 各 musician の座標
  - placements: [{'x': float, 'y': float}]
- 各 musician の音量 (optional; v3 で追加)
  - volumes: float list
    - 0.0 <= volumes[i] <= 10.0 を満たす必要がある

```json
{
    "placements": [
        { "x": 590.0, "y": 10.0 },
        { "x": 1100.0, "y": 100.0 },
        { "x": 1100.0, "y": 150.0 }
    ],
    "volumes": [1.0, 5.0, 3.0]
}
```

### スコアリング

以下の関数を最大化したい。(volumes が未設定の場合、volumes[k]=1.0 とみなす)

<!-- $$\mathrm{Score} = \sum_{\substack{i: \mathrm{attendee}\\k: \mathrm{musician}}} I_i(k)$$ -->
<!-- $$\mathrm{Score} = \sum_{k: \mathrm{musician}}\ \sum_{i: \mathrm{attendee}} \lceil q(k) \cdot I_i(k) \rceil$$ -->
$$\mathrm{Score} = \sum_{k: \mathrm{musician}}\ \sum_{i: \mathrm{attendee}} \lceil \mathrm{volumes}[k] \cdot q(k) \cdot I_i(k) \rceil$$

$I_i(k)$ は以下で定義される。

$$I_i(k) = \left\lceil 10^6 \cdot \frac{\mathrm{attendees}[i].\mathrm{tastes}[\mathrm{mucisians}[k]]}{d^2} \right\rceil$$

ただし、k から i への線が k' を中心とする半径 5 の円と交差する場合、k' は出席者 i に対して k からの音を遮る (交差点が1点からなる場合は音が遮られない。) 音が遮られている場合、$I_i(k) = 0$ となる。

さらに、共演（ルール拡張）によって $q(i)$ が定義される。同じ楽器を演奏する musician 同士でプラスに相互作用が発生する。

$$q(i) = 1 + \sum_{\substack{i \neq j\\\mathrm{musicians}[i]=\mathrm{musicians}[j]}} \frac{1}{d(i,j)}$$

### 制約

musician の配置が間違っていたり、すべての musician が配置されていない解答は認められない。

- 部屋
  - Lambda Band は部屋で演奏する
  - stage は部屋のどこにでも置ける
- musician
  - musician は stage で区切られた領域に配置されなければならない
  - 自分を中心とした半径 10 の円の中に他の musician や stage の端があってはならない（円の周上は OK）
