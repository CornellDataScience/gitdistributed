#pragma once

#include <vector>
#include <string>
#include <memory>
#include "commands.hpp"

// Enum for message types
enum class MessageType : int {
    CLIENT_REQUEST = 0,
    CLIENT_REPLY = 1,
    PRIMARY_REQUEST = 2,
    PRIMARY_REPLY = 3,
    PING = 4,
    VIEW_REPLY = 5,
};

// // Enum for command types within a ClientRequest
// enum class CommandType : int {
//     INIT = 0,
//     ADD = 1,
//     COMMIT = 2,
//     PUSH = 3,
//     PULL = 4
// };

/**
 * @class Message
 * @brief Base class for all network messages.
 *
 * Defines the basic structure and interface for serialization and deserialization.
 */
class Message {
public:
    MessageType type;
    int status = 0; // status field

    // Pure virtual functions to be implemented by subclasses
    Message() = default;
    ~Message() = default;

    // Helper to get message type from a raw buffer before full deserialization
    static MessageType peek_type(const char* data);
    static Message deserialize(const char* data);
    static std::vector<char> serialize(Message &message);
};

/**
 * @class ClientRequest
 * @brief Represents a request from a client to the server.
 */
class ClientRequest : public Message {
public:
    CommandType command_type;
    std::string file_name;
    std::string file_data;

    ClientRequest();
    ClientRequest(CommandType cmd);
    ClientRequest(CommandType cmd, std::string name, std::string data);

    static std::vector<char> serialize(ClientRequest *request);
    static void deserialize(char* data, ClientRequest &request);
};

/**
 * @class ClientReply
 * @brief Represents a reply from the server to a client.
 */
class ClientReply : public Message {
public:
    Command command;

    ClientReply();
    ClientReply(Command cmd);

    static std::vector<char> serialize(ClientReply *reply);
    static void deserialize(char* data, ClientReply &reply);
};

/**
 * @class ViewReply
 * @brief Represents a reply from the viewserver to the client with updated view.
 */
class ViewReply : public Message {
public:
    int view_num;
    std::string primary;
    std::string backup;

    ViewReply();
    explicit ViewReply(int view_num, std::string primary, std::string backup);
    
    static std::vector<char> serialize(ViewReply *reply);
    static void deserialize(char* data, ViewReply &reply);
};

/**
 * @class Ping
 * @brief Represents a ping message from server to viewserver.
 */
class Ping : public Message {
public:
    int view_num;
    std::string server_id;
    
    Ping();
    explicit Ping(int view_num, std::string id);

    static std::vector<char> serialize(Ping *p);
    static void deserialize(char* data, Ping &p);
};
