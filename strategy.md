# 戦略をまとめる

## 同じ楽器同士の人を固めて配置する
- 楽器の重複が少ないケース (1, ...) と、多いケース (6, 7, 10, 30, ...) がある
- どこに、どう固めて置く？
  - q, volumes の項を無視すると、ミュージシャンたちの席だけ固定（誰が座るかは未定）したとき、
  - 各席に対して楽器 x の演奏者を置いたときのうれしさが与えられた状態で、これを最大化する問題になる……①
  - これは寝ながら考える
    - まだ考えてない

## ①
- 貪欲である程度いい解を得られるはず
- 他の人を置いたときと比べて、どれくらい嬉しいか？を指標に上から見ていく

## スコア差分計算高速化
- musician k を移動させたときの影響
  - q(k), k と同じ楽器の人 k' の q(k')
  - k が遮っていた/新たに遮る人 k' の I_i(k')
  - 全員の volumes
- 高速化は難しいかも
  - 差分の近似値で代用して、定期的にキャリブレーションする？（面倒なので後回し）

## 不人気な楽器は音量を 0 にしてしまう
- ステージの真ん中に固めてしまう？
- 好き嫌いの分かれる楽器の隣に置いて、指向性を疑似的に作り出す

## 楽器のグループを最初にふわっと配置する
- 妨害しあう関係が後にならないと分からないので、その部分をどう扱うか

## 山登り：ランダムなミュージシャンをランダムな位置に移動させる
- これ書くか～～～
  - かいた

## 山登り：ランダムなミュージシャン同士を swap する
