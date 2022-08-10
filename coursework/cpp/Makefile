

function:
	./push.sh

exec: entry.cpp *.hpp
	g++ -std=c++17 -o exec entry.cpp \
    -L/usr/local/lib/ -lboost_coroutine -lboost_chrono -lboost_thread -lboost_system -pthread
