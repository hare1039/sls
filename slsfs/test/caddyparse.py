import json
import sys

with open(sys.argv[1]) as f:
    for s in f.readlines():
        j = json.loads(s)
        "{0: <5}".format("abc")
        try:
            print("{0: >14,}: {1}".format(j["ts"], j["request"]["headers"]["Log"]))
        except Exception as e: pass
