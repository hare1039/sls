

function:
	./push.sh

exec: datafunction.cpp *.hpp
	g++ -std=c++17 -o exec datafunction.cpp \
    -L/usr/local/lib/ -lboost_coroutine -lboost_chrono -lboost_thread -lboost_system -pthread

deploy:
	wsk -i action update hare1039/slsfs-actionproxy --docker 'openwhisk/actionloop-base:nightly'

main.zip:
	docker run -i --rm 'action-golang-v1.16:nightly' -compile main <$(SRC) >$(ZIP)
