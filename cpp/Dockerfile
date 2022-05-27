
FROM openwhisk/dockerskeleton

ENV FLASK_PROXY_PORT 8080

RUN apk update && \
    apk add --no-cache --virtual .build-deps \
                                 bzip2-dev \
                                 g++ \
                                 libc-dev \
                                 cassandra-cpp-driver-dev \
                                 boost-dev

### Add source file(s)
ADD * /action/

RUN cd /action && g++ -O3 -std=c++17 -o exec entry.cpp \
    -L/usr/local/lib/ -lboost_coroutine -lboost_chrono -lboost_thread -lboost_system -lcassandra

CMD ["/bin/bash", "-c", "cd actionProxy && python -u actionproxy.py"]
