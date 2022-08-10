
tag=0.0.$(shuf -i 1-100000000 -n 1);
set -e;

sudo docker build -t hare1039/cpp:$tag .
sudo docker push hare1039/cpp:$tag
wsk -i action delete cpp || true;
wsk -i action create cpp --docker hare1039/cpp:$tag

#wsk -i action invoke --blocking cpp --param operation format && \
#    wsk -i action invoke --blocking cpp --param filename /aaaa.txt --param operation write --param data helooooo  && \
#    wsk -i action invoke --blocking cpp --param filename /aaaa.txt --param operation write --param data jack      && \
#    wsk -i action invoke --blocking cpp --param filename /aaaa.txt --param operation read && \
#    wsk -i action invoke --blocking cpp --param filename /bbbb.txt --param operation write --param data llllvkkkk && \
#    wsk -i action invoke --blocking cpp --param filename /bbbb.txt --param operation read


#wsk -i action invoke --blocking cpp --param operation format && \
#    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation touch && \
#    wsk -i action invoke --blocking cpp --param filename /directoy --param operation ls && \
#    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation write --param data goosemonitor && \
#    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation read &&\
#    wsk -i action invoke --blocking cpp --param filename /directoy/dddd.txt --param operation write --param data mothergoose && \
#    wsk -i action invoke --blocking cpp --param filename / --param operation ls && \
#    wsk -i action invoke --blocking cpp --param filename /directoy --param operation ls && \
#    wsk -i action invoke --blocking cpp --param filename /cccc.txt --param operation read && \
#    wsk -i action invoke --blocking cpp --param filename /cccc.txt --param operation write --param data kokoronine && \
#    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation read


wsk -i action invoke --blocking cpp --param operation format --result && \
    wsk -i action invoke --blocking cpp --param filename / --param operation ls --result && \
    wsk -i action invoke --blocking cpp --param filename /directoy --param operation ls --result && \
    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation touch --result && \
    wsk -i action invoke --blocking cpp --param filename /directoy --param operation ls --result && \
    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation write --param data goosemonitor --result && \
    wsk -i action invoke --blocking cpp --param filename /directoy/cccc.txt --param operation read --result
