import argparse
import json
import re

import requests

NUM_PROBLEMS = 90

with open('./token', 'r') as token_file:
    TOKEN = token_file.read().strip()

def api(method, path, data = None):
    headers = {
        'Content-Type': 'application/json',
        'Authorization': f'Bearer {TOKEN}'
    }

    if method == 'get':
        return requests.get('https://api.icfpcontest.com/' + path, headers=headers)

    if method == 'post':
        if data is None:
            return requests.post('https://api.icfpcontest.com/' + path, headers=headers)
        else:
            return requests.post('https://api.icfpcontest.com/' + path, headers=headers, data=json.dumps(data))

    raise valueError('baka')


def submissions():
    resp = api('get', 'submissions?offset=0&limit=1000')
    submissions = resp.json()
    submissions = submissions['Success']
    assert(submissions)

    maxes = dict()
    latests = dict()

    for submission in submissions:
        problem_id = submission['problem_id']
        if ('Success' in submission['score']) and (problem_id not in maxes or (maxes[problem_id]['score']['Success'] < submission['score']['Success'])):
            maxes[problem_id] = submission
        if problem_id not in latests or latests[problem_id]['submitted_at'] < submission['submitted_at']:
            latests[problem_id] = submission

    for problem_id in sorted(latests):
        max_ = maxes[problem_id]['score']
        max_ = max_['Success']

        latest = latests[problem_id]['score']
        latest = latest['Success'] if 'Success' in latest else -1

        print(problem_id, max_, latest)


# usage: python3 src/submit.py --filename output/problem-processed-1.json
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    args = parser.parse_args()
    submissions()