all: client.cpp server.cpp git.cpp parse.cpp
	g++ -std=c++20 -Wall client.cpp -o gitd
	g++ -std=c++20 -Wall server.cpp git.cpp parse.cpp -o server

client: client.cpp
	g++ -std=c++20 -Wall client.cpp -o gitd

server: git.cpp parse.cpp
	g++ -std=c++20 -Wall git.cpp parse.cpp -o server

test: parse_tests.cpp parse.cpp
	g++ -std=c++20 -Wall parse_tests.cpp parse.cpp -o parse_tests

clean:
	rm -f gitd
	rm -f server
	rm -f parse_tests