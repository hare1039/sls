.PHONY: exec-debug exec function function-debug test_swift

function-debug: *.cpp *.hpp exec-debug
	./deploy.sh

function: *.cpp *.hpp exec
	./deploy.sh

exec-debug: *.cpp *.hpp
	DOCKER_BUILDKIT=1 docker build -t hare1039/slsfs-datafunction:nightly \
                                   --build-arg debug=true \
                                   -f Dockerfile .. && \
    docker run --rm hare1039/slsfs-datafunction:nightly cat /action/exec > exec && \
    chmod +x exec

exec: *.cpp *.hpp
	DOCKER_BUILDKIT=1 docker build -t hare1039/slsfs-datafunction:nightly \
                                   -f Dockerfile .. && \
    docker run --rm hare1039/slsfs-datafunction:nightly cat /action/exec > exec && \
    chmod +x exec

test_swift: *.cpp *.hpp
	DOCKER_BUILDKIT=1 docker build -t hare1039/slsfs-datafunction:nightly \
                                   --build-arg debug=true \
                                   -f Dockerfile .. && \
    docker run --rm hare1039/slsfs-datafunction:nightly cat /action/test_swift > test_swift && \
    chmod +x test_swift

deploy:
	wsk -i action update hare1039/slsfs-actionproxy --docker 'openwhisk/actionloop-base:nightly'
