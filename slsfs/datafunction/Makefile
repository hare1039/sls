

function:
	./push.sh

exec: datafunction.cpp *.hpp
	g++ -std=c++17 -o exec datafunction.cpp \
    -L/usr/local/lib/ -lboost_coroutine -lboost_chrono -lboost_thread -lboost_system -pthread
