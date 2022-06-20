
FROM openwhisk/dockerskeleton

ENV FLASK_PROXY_PORT 8080

RUN apk update && \
    apk add --no-cache --virtual .build-deps \
                                 bzip2-dev \
                                 g++ \
                                 libc-dev \
                                 cassandra-cpp-driver-dev \
                                 curl-dev \
                                 make

RUN apk add crypto++-dev --repository=http://dl-cdn.alpinelinux.org/alpine/edge/testing/

RUN git clone --branch v1.8.2 https://github.com/edenhill/librdkafka.git /librdkafka && \
    cd /librdkafka && ./configure --enable-static --install-deps && \
    make && make install

ADD libslsfs /action/libslsfs

ADD datafunction /action/

RUN cd /action && g++ -O3 -std=c++17 -o exec *.cpp \
    -I/action/libslsfs/ -I/action/libslsfs/slsfs -DFMT_HEADER_ONLY \
    -L/usr/local/lib/ \
    -lcryptopp -lcassandra -lrdkafka++ -lrdkafka -lcurl

CMD ["/bin/bash", "-c", "cd actionProxy && python -u actionproxy.py"]
