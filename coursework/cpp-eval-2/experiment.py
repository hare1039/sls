import requests
import urllib3.exceptions
import time
import json
import pprint
import os
import collections
import threading
import os.path
import statistics
import random
import py7zr
import multiprocessing
import pathlib

requests.packages.urllib3.disable_warnings(category=urllib3.exceptions.InsecureRequestWarning)

globalheader = {
    "Authorization": "Basic MjNiYzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOjEyM3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A=",
    "Content-Type": "application/json",
    "User-Agent": "OpenWhisk-CLI/1.0 (2021-04-01T23:49:54.523+0000) linux amd64"
}

gencache = {}

def datagen(size):
    global gencache
    if size in gencache:
        return gencache[size]
    v = ""
    for i in range(size):
        v += "v"

    gencache[size] = v
    return v

class testcase:
    def __init__(self, actionimage, testname):
        self.result = []
        self.actionimage = actionimage
        self.testname = testname

    def inits(self, writesize): pass
    def tests(self, writesize): pass

    def reset(self):
        self.result = []

    def save(self):
        with open(self.id() + ".json", "w") as fp:
            json.dump(json.dumps(self.result), fp)

    def load(self):
        with open(self.id() + ".json", "r") as fp:
            self.result = json.load(fp)

    def post(self, data):
        global globalheader
        return requests.post("https://172.17.0.1/api/v1/namespaces/_/actions/{}?blocking=true".format(self.actionimage),
                             verify=False,
                             data=json.dumps(data),
                             headers=globalheader)

    def record(self, data, sigature):
        start = time.perf_counter()
        res = self.post(data)
        duration = time.perf_counter() - start

        v = res.json()
        k = {}
        k["perf_counter"] = duration * 1000
        k["sigature"] = sigature
        k["response"] = "response" in v
        k["duration"] = v["duration"] if "duration" in v else False
        del v
        self.result.append(k)

    def getresultkey(self, writesize, memo):
        return "{}_{}_writesize-{}".format(self.id(), memo, writesize)

    def id(self):
        return "test-{}_img-{}".format(self.testname, self.actionimage)

    def summary(self):
        return self.result;

class writefile(testcase):
    def __init__(self, actionimage):
        super().__init__(actionimage, "writefile")

    def inits(self, writesize):
        self.post({"filename": "/", "operation": "format"})

    def tests(self, writesize):
        self.record({"filename": "/aaaa.txt",
                     "operation": "write",
                     "data": datagen(writesize)},
                    "write1")

class readfile(testcase):
    def __init__(self, actionimage):
        super().__init__(actionimage, "readfile")

    def inits(self, writesize):
        self.post({"filename": "/", "operation": "format"})
        self.post({"filename": "/aaaa.txt", "operation": "write", "data": datagen(writesize)})

    def tests(self, writesize):
        self.record({"filename": "/aaaa.txt",
                     "operation": "read"},
                    "read1")

class writereadfile(testcase):
    def __init__(self, actionimage):
        super().__init__(actionimage, "writereadfile")

    def inits(self, writesize):
        self.post({"filename": "/", "operation": "format"})
        self.post({"filename": "/aaaa.txt", "operation": "touch"})

    def tests(self, writesize):
        self.record({"filename": "/aaaa.txt",
                     "operation": "write",
                     "data": datagen(writesize)},
                    "write1")
        self.record({"filename": "/aaaa.txt",
                     "operation": "read"},
                    "read1")

class mkdirtest(testcase):
    def __init__(self, actionimage, pathlen):
        super().__init__(actionimage, "mkdirtest+pathlen{}".format(pathlen))
        self.namepool = ["folder01", "folder02", "folder03", "folder04"]
        self.pathlen = pathlen

    def inits(self, writesize):
        self.post({"filename": "/", "operation": "format"})
        self.post({"filename": "/folder00", "operation": "mkdir"})

    def tests(self, writesize):
        filepath = []
        for i in range(self.pathlen):
            filepath.append(random.choice(self.namepool))

        now = ""
        counter = 0
        for p in filepath:
            now += ("/" + p)
            self.record({"filename": now,
                         "operation": "mkdir"},
                        "mkdir{}".format(counter))
            counter += 1

def runtest(test, repeat, writesize, memo, arrayid, return_array):
    test.inits(writesize)
    start = time.perf_counter()
    for r in range(repeat):
        test.tests(writesize)
        duration = time.perf_counter() - start
        if duration >= 60:#2 * 50:
            break;

    return_array[arrayid] = {
        "duration": duration * 1000, # second -> millisecond
        "key": test.getresultkey(writesize, memo),
        "result": test.summary()
    }
    test.reset()

#manager = multiprocessing.Manager()
#return_dict = manager.dict()
return_array = []

def summarize(resultdict):
    result = collections.defaultdict(lambda: {
        "latency": collections.defaultdict(lambda: {
            "internal": 0.0,
            "external": 0.0,
            "total": 1.0,
            "success": 1.0
        }),
        "throughput": 0.0
    })

    for k, v in resultdict.items():
        operation = set()
        for single_thread_result in v:
            k = single_thread_result["key"]

            for req in single_thread_result["result"]:
                result[k]["latency"][req["sigature"]]["total"] += 1
                if req["response"] == False:
                    continue
                result[k]["latency"][req["sigature"]]["external"] += req["perf_counter"]
                result[k]["latency"][req["sigature"]]["internal"] += req["duration"]
                result[k]["latency"][req["sigature"]]["success"] += 1

                operation.add(req["sigature"])

            totalexternal = 0.0
            totalinternal = 0.0
            totalcount = 0.0
            totalsuccess = 0.0
            for op in operation:
                result[k]["latency"][op]["external"] /= result[k]["latency"][op]["success"]
                totalexternal += result[k]["latency"][op]["external"]
                result[k]["latency"][op]["internal"] /= result[k]["latency"][op]["success"]
                totalinternal += result[k]["latency"][op]["internal"]
                totalcount += result[k]["latency"][op]["total"]
                totalsuccess += result[k]["latency"][op]["success"]

            result[k]["latency"]["total"]["external"] = totalexternal
            result[k]["latency"]["total"]["internal"] = totalinternal
            result[k]["latency"]["total"]["total"] = totalcount
            result[k]["latency"]["total"]["success"] = totalsuccess
            #print(k, result[k]["latency"]["total"])

    for k, v in resultdict.items():
        for single_thread_result in v:
            k = single_thread_result["key"]
            result[k]["throughput"] += len(single_thread_result["result"]) / single_thread_result["duration"]

    return result

def is_7z_valid(target):
    if not py7zr.is_7zfile(target):
        return False

    with open(target, "rb") as f:
        try:
            a = py7zr.SevenZipFile(f)
            if a.testzip() is None:
                return True
            else:
                return False
        except py7zr.exceptions.Bad7zFile:
            return False
        except py7zr.exceptions.PasswordRequired:
            return False
    return False

def compress(jsonfile):
    zipname = "{}.7z".format(jsonfile)
    if os.path.exists(zipname) and is_7z_valid(zipname):
        with open(jsonfile, "r+") as fp:
            fp.truncate(0)
        return

    os.system("7z a -y -t7z -mx=9 -mfb=273 -ms -md=31 -myx=9 -mtm=- -mmt -mmtf -md=1536m -mmf=bt3 -mmc=100 -mpb=0 -mlc=0 -m0=LZMA2:27 {} {}".format(zipname, jsonfile))
    if is_7z_valid(zipname):
        with open(jsonfile, "r+") as fp:
            fp.truncate(0)

def gen_result(jsonfile):
    if os.path.getsize(jsonfile) == 0:
        os.system("7z x -y {}.7z".format(jsonfile))

    with open(jsonfile) as fp:
        data = json.load(fp)
        result = summarize(data)
        with open(test.id() + "-result.json", "w") as fp:
            json.dump(result, fp)
        print(result)
        del data

    compress(jsonfile)


if __name__ == "__main__":
    #tests = [writefile("cpp"), ]
    #tests = [writefile("cpp"), readfile("cpp"), writereadfile("cpp")]
    tests = [writefile("cpp"), readfile("cpp"), writereadfile("cpp"), mkdirtest("cpp", 1), mkdirtest("cpp", 6)]
    #tests = [writereadfile("cpp")]
    writesizes = [16, 256, 1024, 4096, 8096]
    client_count = [1, 16, 64, 128]
    repeat = 10000

    worker_processes = []
    for test in tests:
        if os.path.exists(test.id() + ".json"):
            #p = multiprocessing.Process(target=gen_result, args=(test.id() + ".json", ))
            #p.start()
            #worker_processes.append(p)
            with open(test.id() + ".json") as fp:
                data = json.load(fp)

            #result = summarize(data)
            #with open(test.id() + "-result.json", "w") as fp:
            #    json.dump(result, fp)
            continue
        elif os.path.exists(test.id() + "-result.json"):
            continue
        os.system("/bin/bash -c './restart_openwhisk.sh'")

        final_dict = {}
        for w in writesizes:
            for c in client_count:
                print("starting test", test.id(), "with wsize =", w, "cli =", c)
                processes = []
                return_array = list(range(c))
                for i in range(c):
                    #p = multiprocessing.Process(target=runtest, args=(test, repeat, w, "client-{}".format(c), return_dict))
                    p = threading.Thread(target=runtest, args=(test, repeat, w, "client-{}".format(c), i, return_array))
                    p.start()
                    processes.append(p)

                for p in processes:
                    p.join()

                final_dict["test-{}-{}-{}".format(test.id(), w, c)] = return_array

        with open(test.id() + ".json", "w") as fp:
            json.dump(final_dict, fp)

        #result = summarize(final_dict)
        #with open(test.id() + "-result.json", "w") as fp:
        #    json.dump(result, fp)
        #print(result)
        del final_dict

        #p = multiprocessing.Process(target=gen_result, args=(test.id() + ".json", ))
        #p.start()
        #worker_processes.append(p)

#    for p in worker_processes:
#        p.join()
