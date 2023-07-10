import argparse
import json
import re

import requests


def send_file_contents(filename):
    
    ##### header #####

    with open('./token', 'r') as token_file:
        token = token_file.read()

    headers = {
        'Content-Type': 'application/json',
        'Authorization': f'Bearer {token}'
    }

    #####  body  #####
    
    with open(filename, 'r') as file:
        file_contents = file.read()

    match = re.search(r'\d+', filename)
    if match:
        problem_id = int(match.group(0))
    else:
        raise ValueError('Filename must contain a number')
    data = {
        'problem_id': problem_id,
        'contents': file_contents
    }

    #####  send  #####
    
    requests.post('https://api.icfpcontest.com/submission', headers=headers, data=json.dumps(data))


# usage: python3 src/submit.py --filename output/problem-processed-1.json
if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('filename')
    args = parser.parse_args()
    send_file_contents(args.filename)