import statistics
import sys
import unicodedata
import json

class onetest:
    def __init__(self):
        self.target = ""
        self.connection = 0
        self.duration = 0
        self.total_request = 0
        self.error_request = 0
        self.connection_error = {}

        self.latency = 0
        self.latency_stdev = 0
        self.latency_max = 0
        self.latency_distribution = {}

        self.thread_throughput = 0
        self.thread_throughput_stdev = 0
        self.thread_throughput_max = 0

        self.request_throughput = 0
        self.network_throughput = 0

        self.responses = []

    def show(self):
        print("target: {}".format(self.target))
        print("connection: {}".format(self.connection))
        print("duration: {}".format(self.duration))
        print("total_request: {}".format(self.total_request))
        print("error_request: {}".format(self.error_request))
        print("connection_error: {}".format(self.connection_error))
        print("latency: {}".format(self.latency))
        print("latency_stdev: {}".format(self.latency_stdev))
        print("latency_distribution: {}".format(self.latency_distribution))
        print("thread_throughput: {}".format(self.thread_throughput))
        print("thread_throughput_stdev: {}".format(self.thread_throughput_stdev))
        print("thread_throughput_max: {}".format(self.thread_throughput_max))
        print("request_throughput: {}".format(self.request_throughput))
        print("network_throughput: {}".format(self.network_throughput))

def to_ms(x):
    if "ms" in x:
        return float(x[:-2])
    elif "us" in x:
        return float(x[:-2]) / 1000
    elif "m" in x and "s" in x:
        l = x.split("m")
        return float(l[0]) * 60 * 1000 + float(l[1][:-1]) * 1000
    elif "s" in x:
        return float(x[:-1]) * 1000

def to_number(x):
    if x.endswith("k"):
        return float(x[:-1]) * 1000
    return float(x)



def filter_non_printable(str):
    printable = {"Lu", "Ll"}
    return "".join(c for c in str if unicodedata.category(c) in printable)


def select(l, pred):
    return [x for x in l if pred(x)]

def transform(l, func):
    for x in l:
        try:
            if isinstance(func, list):
                for f in func:
                    x = f(x)
                yield x
            else:
                yield func(x)
        except Exception as e:
            pass

def is_json(string):
    try:
        return json.loads(string)
    except ValueError as e:
        return None


def goodmean(l):
    try:
        return round(statistics.mean(l), 2)
    except:
        return 0

def goodstdev(l):
    try:
        return round(statistics.stdev(l), 2)
    except Exception as e:
        return 0

with open(sys.argv[1]) as fp:
    lines = fp.readlines()

    tests = []

    t = onetest()
    state = None
    state_object = None

    for i in range(len(lines)):
        line = lines[i]

        # 50%   30.38ms
        # 75%   40.13ms
        # 90%   69.75ms
        # 99%   5.05s
        if state == "LatencyDistributionState":
            l = line.split()
            t.latency_distribution[l[0]] = to_ms(l[1])
            state_object += 1
            if state_object == 4:
                state = None
                state_object = None
            continue

        # Running 50s test @ https://172.17.0.1/api/v1/namespaces/_/actions/direct-dev-hello?blocking=true&result=true
        if line.startswith("Running"):
            if t.total_request != 0:
                tests.append(t)
            l = line.split()
            t = onetest()
            t.target = l[4]
            t.duration = to_ms(l[1])

        # 40 threads and 100 connections
        elif "connections" in line and "threads" in line:
            l = line.split()
            t.connection = int(l[3])

        # invoker {"invoker0/0":"up","invoker1/1":"up","invoker2/2":"up"}
        elif line.startswith("invoker"):
            if line != 'invoker {"invoker0/0":"up","invoker1/1":"up","invoker2/2":"up"}':
                print(line)

        # 502 {"error":"The action exhausted its memory and was aborted."}
        elif line.startswith("502"):
            pass

        # Thread Stats   Avg      Stdev     Max   +/- Stdev
        elif line.startswith("  Thread"):
            pass

        # Latency   212.88ms  903.82ms  12.14s    95.60%
        elif line.startswith("    Latency"):
            l = line.split()
            t.latency = to_ms(l[1])
            t.latency_stdev = to_ms(l[2])
            t.latency_max = to_ms(l[3])

        # Req/Sec    57.00     18.76   110.00     68.92%
        elif line.startswith("    Req/Sec"):
            l = line.split()
            t.thread_throughput = to_number(l[1])
            t.thread_throughput_stdev = to_number(l[2])
            t.thread_throughput_max = to_number(l[3])

        #   Latency Distribution
        elif line.startswith("  Latency Distribution"):
            state = "LatencyDistributionState"
            state_object = 0

        # 110815 requests in 50.05s, 69.75MB read
        elif "requests" in line and "read" in line:
            l = line.split()
            t.total_request = int(l[0])
            if t.total_request == 0:
                print("warning: t.total_request == 0")

        # Non-2xx or 3xx responses: 41
        elif line.startswith("  Non-2xx or 3xx responses"):
            l = line.split()
            t.error_request = int(l[4])

        # 200 {}
        elif line.lstrip().startswith("200"):
            l = line[len("200"):]
            try:
                t.responses.append(json.loads(l[1]))
            except Exception as e:
                continue

        #   Socket errors: connect 3459, read 8069, write 0, timeout 0
        elif line.startswith("  Socket errors"):
            l = line.split()
            t.connection_error = {
                "connect": int(l[3][:-1]),
                "read": int(l[5][:-1]),
                "write": int(l[7][:-1]),
                "timeout": int(l[9])
            }

        # Requests/sec:   2214.30
        elif line.startswith("Requests/sec"):
            l = line.split()
            t.request_throughput = to_number(l[1])

        # Transfer/sec:      1.39MB
        elif line.startswith("Transfer/sec"):
            l = line.split()
            t.network_throughput = l[1]

        # 500 <html>
        elif line.startswith("500") or line.startswith("<") or line == "":
            pass

        else:
#            line = "".join(filter(lambda x: x in string.printable, line))
            if filter_non_printable(line) == "":
                continue
            elif is_json(line):
                t.responses.append(json.loads(line))
                continue
            elif line.lstrip().startswith("{"):
                continue

            print("not parsed:", line)

if t.total_request != 0:
    tests.append(t)

try:
    startstep = list(transform(tests, [
        lambda x: x.responses,
        lambda responses: goodmean(transform(responses, [
            lambda response: response["response"]["result"]["step1_start"] - response["start"]
        ]))
    ]))
    print("startstep(ms):", goodmean(startstep), "+/-", goodstdev(startstep))

    step1time = list(transform(tests, [
        lambda x: x.responses,
        lambda responses: goodmean(transform(responses, [
            lambda response: response["response"]["result"],
            lambda result: result["step1_end"] - result["step1_start"]
        ]))
    ]))
    print("step1time(ms):", goodmean(step1time), "+/-", goodstdev(step1time))

    step1delta = list(transform(tests, [
        lambda x: x.responses,
        lambda responses: goodmean(transform(responses, [
            lambda response: response["response"]["result"],
            lambda result: result["step2_start"] - result["step1_end"]
        ]))
    ]))
    print("step1delta(ms):", goodmean(step1delta), "+/-", goodstdev(step1delta))

    step2time = list(transform(tests, [
        lambda x: x.responses,
        lambda responses: goodmean(transform(responses, [
            lambda response: response["response"]["result"],
            lambda result: result["step2_end"] - result["step2_start"]
        ]))
    ]))
    print("step2time(ms):", goodmean(step2time), "+/-", goodstdev(step2time))

    step2delta = list(transform(tests, [
        lambda x: x.responses,
        lambda responses: goodmean(transform(responses, [
            lambda response: response["response"]["result"],
            lambda result: result["step3_start"] - result["step2_end"]
        ]))
    ]))
    print("step2delta(ms):", goodmean(step2delta), "+/-", goodstdev(step2delta))

    step3time = list(transform(tests, [
        lambda x: x.responses,
        lambda responses: goodmean(transform(responses, [
            lambda response: response["response"]["result"],
            lambda result: result["step3_end"] - result["step3_start"]
        ]))
    ]))
    print("step3time(ms):", goodmean(step3time), "+/-", goodstdev(step3time))

    endstep = list(transform(tests, [
        lambda x: x.responses,
        lambda responses: goodmean(transform(responses, [
            lambda response: response["end"] - response["response"]["result"]["step3_end"]
        ]))
    ]))
    print("endstep(ms):", goodmean(endstep), "+/-", goodstdev(endstep))

except Exception as e:
    print("steptime Exception", e)


l = list(transform(tests, lambda x: x.request_throughput))
print("throughput (req/sec):", goodmean(l), "+/-", goodstdev(l))

l = list(transform(tests, lambda x: x.latency))
print("latency (ms):", goodmean(l), "+/-", goodstdev(l))

l = list(transform(tests, lambda x: x.latency_distribution["50%"]))
print("latency (50%):", goodmean(l), "+/-", goodstdev(l))

l = list(transform(tests, lambda x: x.latency_distribution["75%"]))
print("latency (75%):", goodmean(l), "+/-", goodstdev(l))

l = list(transform(tests, lambda x: x.latency_distribution["90%"]))
print("latency (90%):", goodmean(l), "+/-", goodstdev(l))

l = list(transform(tests, lambda x: x.latency_distribution["99%"]))
print("latency (99%):", goodmean(l), "+/-", goodstdev(l))

l = list(transform(tests, lambda x: x.error_request / x.total_request))
print("error rate:", round(statistics.mean(l), 6), "+/-", goodstdev(l))

try:
    l = list(transform(tests, lambda x: {
        "connect": x.connection_error["connect"] / x.total_request,
        "read":    x.connection_error["read"]    / x.total_request,
        "write":   x.connection_error["write"]   / x.total_request,
        "timeout": x.connection_error["timeout"] / x.total_request
    }))

    print("connection error rate:")
    print("  connect:", round(goodmean(transform(l, lambda x: x["connect"]))), "+/-", goodstdev(transform(l, lambda x: x["connect"])))
    print("  read:   ", round(goodmean(transform(l, lambda x: x["read"]))),    "+/-", goodstdev(transform(l, lambda x: x["read"])))
    print("  write:  ", round(goodmean(transform(l, lambda x: x["write"]))),   "+/-", goodstdev(transform(l, lambda x: x["write"])))
    print("  timeout:", round(goodmean(transform(l, lambda x: x["timeout"]))), "+/-", goodstdev(transform(l, lambda x: x["timeout"])))

except Exception as e: pass
