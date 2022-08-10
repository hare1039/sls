#!/bin/bash

tag=nightly;
set -e;

docker build -t hare1039/slsfs-datafunction:$tag -f Dockerfile ..
#docker push hare1039/slsfs-datafunction:$tag

if [[ "$BUILDONLY" == "TRUE" ]]; then
    exit 0;
fi

#ssh zion08 docker pull hare1039/slsfs-datafunction:$tag &
#ssh zion09 docker pull hare1039/slsfs-datafunction:$tag &
#ssh zion10 docker pull hare1039/slsfs-datafunction:$tag &

docker run --rm hare1039/slsfs-datafunction:nightly cat /action/exec > exec
#docker run --rm --name dfcopy -d hare1039/slsfs-datafunction:nightly bash -c 'sleep 100';
#docker cp dfcopy:/action/exec .;
#docker cp dfcopy:/action/libs go-image;
#docker stop dfcopy;

strip exec

#wait;

#rm -f action.zip
#zip -r action.zip exec
#
#wsk -i action delete slsfs-datafunction || true;
##wsk -i action create slsfs-datafunction --kind slsfs --native action.zip --concurrency 1000;
#wsk -i action create slsfs-datafunction --kind go:1.15 --native action.zip --concurrency 1000;
#
#wsk -i action delete slsfs-metadatafunction || true;
##wsk -i action create slsfs-metadatafunction --kind slsfs --native action.zip --concurrency 1000;
#wsk -i action create slsfs-metadatafunction --kind go:1.15 --native action.zip --concurrency 1000;
#
#rm -f action.zip exec;

ssh zion08 docker restart invoker0 &
ssh zion09 docker restart invoker1 &
ssh zion10 docker restart invoker2 &

zip -r hello.zip exec
wsk -i action update slsfs-datafunction hello.zip --docker openwhisk/actionloop-base --concurrency 1000;
wsk -i action update slsfs-metadatafunction hello.zip --docker openwhisk/actionloop-base --concurrency 1000;

rm -f exec hello.zip;

wait

#wsk -i action invoke --blocking slsfs-datafunction --param operation create --param filename / --param type metadata
#
#echo "start create /helloworld.txt"
#wsk -i action invoke --blocking slsfs-datafunction --param operation create --param filename /helloworld.txt --param type metadata
./test.sh

#echo "start read   /helloworld.txt"
#wsk -i action invoke --blocking slsfs-datafunction --param operation read --param filename /helloworld.txt --param type file
#
#echo "start ls /"
#wsk -i action invoke --blocking slsfs-datafunction --param operation ls --param filename / --param type metadata

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
