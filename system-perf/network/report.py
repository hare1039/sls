
import sys
import csv


def rollout(final, name):
    with open(name) as f:
        lines = f.readlines()

        hostname = name.split("_")[0]
        size = name.split("_")[1].split(".")[0]

        counter = 0

        for l in lines:
            if "0.00-30.00" in l and "sender" in l:
                d = l.split()
                data = d[6] + d[7]

                if counter == 0:
                    ttype = "hare send"
                elif counter == 1:
                    ttype = "hare recv"
                elif counter == 2:
                    ttype = "hare send-bidir"
                elif counter == 3:
                    ttype = "hare recv-bidir"
                final.append( (hostname, size, ttype, data) )
                counter += 1

net = []

for f in sys.argv[1:]:
    rollout(net, f)

#('zion10', '8K', 3, '6.21Gbits/sec')
#hosts = sorted(list(set([x[0] for x in net])))
#types = sorted(list(set([x[1] for x in net])))

with open("result-haresend.csv", 'w') as f:
    mywriter = csv.writer(f, delimiter=',')
    mywriter.writerows(filter(lambda x: x[2] == "hare send", net))

with open("result-harerecv.csv", 'w') as f:
    mywriter = csv.writer(f, delimiter=',')
    mywriter.writerows(filter(lambda x: x[2] == "hare recv", net))

with open("result-bidir.csv", 'w') as f:
    mywriter = csv.writer(f, delimiter=',')
    mywriter.writerows(filter(lambda x: "bidir" in x[2], net))

#print(net)
