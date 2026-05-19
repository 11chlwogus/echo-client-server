all:
	g++ -std=c++11 -pthread -o echo-server echo-server.cpp
	g++ -std=c++11 -pthread -o echo-client echo-client.cpp

clean:
	rm -f echo-server echo-client
