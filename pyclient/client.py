import requests
import time
import urllib3

urllib3.disable_warnings()

def runfunction(target, myobj, times):
    url = "https://172.17.0.1/api/v1/namespaces/_/actions/{}?blocking=true&result=false".format(target)
    headers = {
        "Authorization": "Basic MjNiYzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOjEyM3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A=",
        "Content-Type": "application/json",
        "User-Agent": "amd64"
    }

    for warm in range(0, 5):
        x = requests.post(url, json = myobj, headers = headers, verify=False)
        #print(x.text)


    start = time.perf_counter_ns()
    for i in range(0, times):
        x = requests.post(url, json = myobj, headers = headers, verify=False)
        #print(x.text)
        if not x.ok:
            print(x.text)
            break
    dur = time.perf_counter_ns() - start
    print(x.text)
    return dur / times

#print(runfunction("direct-dev-hello", {}, 1000))

#print(runfunction("hellozip", {"name":"World"}, 1000))
#print(runfunction("helloPython", {"name":"World"}, 1000))
#print(runfunction("hello-go", {"name":"World"}, 1000))

print(runfunction("slsfs-datafunction", {"operation": "read", "filename": "/helloworld.txt", "type": "file"}, 800))
#print(runfunction("slsfs-datafunction", {"operation": "write", "filename": "/helloworld.txt", "type": "file", "data": "osdiatc2022"}, 100))
