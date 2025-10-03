all: client.cpp server.cpp
	g++ -std=c++20 -Wall client.cpp -o gitd
	g++ -std=c++20 -Wall server.cpp -o server

client: client.cpp
	g++ -std=c++20 -Wall client.cpp -o gitd

server: server.cpp
	g++ -std=c++20 -Wall server.cpp -o server

clean:
	rm -f gitd
	rm -f server