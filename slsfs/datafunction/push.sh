
tag=0.0.$(shuf -i 1-100000000 -n 1);
set -e;

sudo docker build -t hare1039/slsfs-datafunction:$tag -f Dockerfile ..
sudo docker push hare1039/slsfs-datafunction:$tag
wsk -i action delete slsfs-datafunction || true;
wsk -i action create slsfs-datafunction --docker hare1039/slsfs-datafunction:$tag

echo "start create /"
wsk -i action invoke --blocking slsfs-datafunction --param operation create --param filename / --param type metadata

echo "start create /hello.txt"
wsk -i action invoke --blocking slsfs-datafunction --param operation create --param filename /hello.txt --param type metadata

echo "start write  /hello.txt"
wsk -i action invoke --blocking slsfs-datafunction --param operation write --param filename /hello.txt --param data paripikoumei --param type file

echo "start read   /hello.txt"
wsk -i action invoke --blocking slsfs-datafunction --param operation read --param filename /hello.txt --param type file

echo "start ls /"
wsk -i action invoke --blocking slsfs-datafunction --param operation ls --param filename / --param type metadata

#wsk -i action invoke --blocking slsfs-datafunction \
#    --param type metadata \
#    --param operation addnewfile \
#    --param filename / \
#    --param data hello/ \
#    --param returnchannel straw

#wsk -i action invoke --blocking slsfs-datafunction \
#    --param type metadata \
#    --param operation addnewfile \
#    --param filename /hello/ \
#    --param data john.txt
#
#wsk -i action invoke --blocking slsfs-datafunction \
#    --param type metadata \
#    --param operation ls \
#    --param filename /hello/

#    wsk -i action invoke --blocking cpp --param filename / --param operation ls --result && \
#    wsk -i action invoke --blocking cpp --param filename /directoy --param operation ls --result && \
#    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation touch --result && \
#    wsk -i action invoke --blocking cpp --param filename /directoy --param operation ls --result && \
#    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation write --param data goosemonitor --result && \
#    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation read --result
