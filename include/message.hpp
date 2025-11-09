#pragma once

#include <vector>
#include <string>
#include <memory>

// Enum for message types
enum class MessageType : int {
    CLIENT_REQUEST = 0,
    CLIENT_REPLY = 1,
    PRIMARY_REQUEST = 2,
    PRIMARY_REPLY = 3,
    PING = 4
};

// Enum for command types within a ClientRequest
enum class CommandType : int {
    INIT = 0,
    ADD = 1,
    COMMIT = 2,
    PUSH = 3,
    PULL = 4
};

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
    virtual ~Message() = default;
    virtual std::vector<char> serialize();
    virtual void deserialize(const std::vector<char>& data) = 0;

    // Helper to get message type from a raw buffer before full deserialization
    static MessageType peek_type(const std::vector<char>& data);
};

/**
 * @class ClientRequest
 * @brief Represents a request from a client to the server.
 */
class ClientRequest : public Message {
public:
    CommandType command_type;
    std::string file_name;
    std::vector<char> file_data;

    ClientRequest();
    ClientRequest(CommandType cmd);
    ClientRequest(CommandType cmd, std::string name, std::vector<char> data);

    std::vector<char> serialize() override;
    void deserialize(const std::vector<char>& data) override;
};

/**
 * @class ClientReply
 * @brief Represents a reply from the server to a client.
 */
class ClientReply : public Message {
public:
    std::string reply_message;

    ClientReply();
    explicit ClientReply(std::string message);

    std::vector<char> serialize() override;
    void deserialize(const std::vector<char>& data) override;
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
    
    std::vector<char> serialize() override;
    void deserialize(const std::vector<char>& data) override;
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

    std::vector<char> serialize() override;
    void deserialize(const std::vector<char>& data) override;
};
