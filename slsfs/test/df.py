
import requests
import time
import urllib3

urllib3.disable_warnings()

def init():
    url = "https://172.17.0.1/api/v1/namespaces/_/actions/slsfs-datafunction?blocking=true&result=false"
    #myobj = {"filename":"/","operation":"create","type":"metadata"}
    myobj = {"filename":"/hello.txt","operation":"create","type":"data"}
    headers = {
        "Authorization": "Basic Nzg5YzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOmFiY3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A=",
        "Content-Type": "application/json",
        "User-Agent": "amd64"
    }

    x = requests.post(url, json = myobj, headers = headers, verify=False)
    print(x.text)

def testone():
    url = "https://172.17.0.1/api/v1/namespaces/_/actions/slsfs-datafunction?blocking=true&result=false"
    #myobj = {"filename":"/","operation":"create","type":"metadata"}
    myobj = {"filename":"/helloworld.txt","operation":"write","type":"file", "data":"orange"}
    headers = {
        "Authorization": "Basic Nzg5YzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOmFiY3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A=",
        "Content-Type": "application/json",
        "User-Agent": "amd64"
    }

    start = time.perf_counter_ns()
    x = requests.post(url, json = myobj, headers = headers, verify=False)
    dur = time.perf_counter_ns() - start
    print(x.text)
    return dur

def testone2():
    url = "https://172.17.0.1/api/v1/namespaces/_/actions/slsfs-datafunction?blocking=true&result=false"
    #myobj = {"filename":"/","operation":"create","type":"metadata"}
    myobj = {"filename":"/helloworld.txt","operation":"read","type":"file"}
    headers = {
        "Authorization": "Basic Nzg5YzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOmFiY3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A=",
        "Content-Type": "application/json",
        "User-Agent": "amd64"
    }

    start = time.perf_counter_ns()
    x = requests.post(url, json = myobj, headers = headers, verify=False)
    dur = time.perf_counter_ns() - start
    return dur, x.json()

def testone3():
    url = "https://172.17.0.1/api/v1/namespaces/_/actions/example-app-slsfs?blocking=true&result=false"
    #myobj = {"filename":"/","operation":"create","type":"metadata"}
    myobj = {"data":"pink lemonade"}

    headers = {
        "Authorization": "Basic Nzg5YzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOmFiY3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A=",
        "Content-Type": "application/json",
        "User-Agent": "amd64"
    }

    start = time.perf_counter_ns()
    x = requests.post(url, json = myobj, headers = headers, verify=False)
    dur = time.perf_counter_ns() - start
    print(x.text)
    return dur

#init()
to = 0;
ti = 0;
for t in range(100):
    r, ri = testone2()
    print(ri["activationId"], r, ri["duration"]*1000000)
    to += r
    ti += (ri["duration"]*1000000)
print("outer dur", to/100)
print("inner dur", ti/100)
