CLIENT_SRCS = clientserver/client.cpp clientserver/commands.cpp clientserver/tcp.cpp clientserver/message.cpp
SERVER_SRCS = clientserver/server.cpp clientserver/commands.cpp clientserver/tcp.cpp gitapp/git_app.cpp clientserver/message.cpp

all: client server

client: $(CLIENT_SRCS)
	g++ -std=c++20 -Wall -Iinclude $(CLIENT_SRCS) -o gitd

server: $(SERVER_SRCS)
	g++ -std=c++20 -Wall -Iinclude $(SERVER_SRCS) -o server

test: serialization_tests.cpp clientserver/tcp.cpp clientserver/messages.cpp
	g++ -std=c++20 -Wall -Iinclude serialization_tests.cpp clientserver/tcp.cpp clientserver/commands.cpp -o serialization_tests

clean:
	rm -f gitd
	rm -f server
	rm -f serialization_tests