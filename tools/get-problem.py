import argparse
import json

import requests


def get_problem(problem_id):
    url = f"https://api.icfpcontest.com/problem?problem_id={problem_id}"

    response = requests.get(url)

    if response.status_code == 200:
        data = response.json()
        data = eval(data['Success'])
        # ファイルに保存する
        filename = f"input/problem-{problem_id}.json"
        with open(filename, "w") as file:
            json.dump(data, file)
        print(f"問題データを {filename} に保存しました。")
    else:
        print("リクエストが失敗しました。")


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('problem_id')
    args = parser.parse_args()
    get_problem(args.problem_id)