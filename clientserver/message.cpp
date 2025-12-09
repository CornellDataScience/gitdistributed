#include "message.hpp"
#include <cstring>
#include <stdexcept>
#include <iostream>

// Appends any serializable data to a vector of chars
template<typename T>
void append_to_buffer(std::vector<char>& buffer, const T& value) {
    const char* value_ptr = reinterpret_cast<const char*>(&value);
    buffer.insert(buffer.end(), value_ptr, value_ptr + sizeof(T));
}

/**
 * Appends a variable-length string to the buffer.
 *
 * It handles the variable length by first writing the string's size to the
 * buffer, followed by the actual characters of the string. This allows the
 * receiving end to know exactly how many characters to read for the string.
 *
 * @param buffer The byte buffer to append to.
 * @param str The string to append.
 */
void append_to_buffer(std::vector<char>& buffer, const std::string& str) {
    int str_len = str.length();
    append_to_buffer(buffer, str_len);
    buffer.insert(buffer.end(), str.begin(), str.end());
}

// Message 
MessageType Message::peek_type(const char* data) {
    if (strlen(data) < sizeof(MessageType)) {
        throw std::runtime_error("Buffer too small to peek message type");
    }
    MessageType type;
    std::memcpy(&type, data, sizeof(MessageType));
    return type;
}

std::vector<char> Message::serialize(Message &message) {
    MessageType message_type = message.type;
    std::vector<char> serialized;
    if (message_type == MessageType::CLIENT_REQUEST) {
        serialized = ClientRequest::serialize((ClientRequest *) &message);
    }
    else if(message_type == MessageType::CLIENT_REPLY){
        serialized = ClientReply::serialize((ClientReply *) &message);
    }
    else if(message_type == MessageType::PING){
        serialized = Ping::serialize((Ping *) &message);
    }
    else if(message_type == MessageType::VIEW_REPLY){
        serialized = ViewReply::serialize((ViewReply *) &message);
    }

    return serialized;
}

// Message Message::deserialize(const char* data) {
//     MessageType message_type = Message::peek_type(data);
//     Message* msg;
    
//     if (message_type == MessageType::CLIENT_REQUEST) {
//         msg = new ClientRequest();
//         ClientRequest::deserialize(data, *msg);
//     } else if (message_type == MessageType::CLIENT_REPLY) {
//         msg = new ClientReply();ClientRequest request = ClientRequest
//     }
    
//     return &msg;}

// ClientRequest
ClientRequest::ClientRequest() {
    this->type = MessageType::CLIENT_REQUEST;
}

ClientRequest::ClientRequest(CommandType cmd) :
    command_type(cmd),
    file_name(""),
    file_data({})
{
    this->type = MessageType::CLIENT_REQUEST;
}

ClientRequest::ClientRequest(CommandType cmd, std::string name, std::string data) {
    this->type = MessageType::CLIENT_REQUEST;
    this->command_type = cmd;
    this->file_name = std::move(name);
    this->file_data = std::move(data);
}

//
// Serializes the ClientRequest object into a byte vector.
// Format: [type | commandType | file_name_size | file_name | file_data_size | file_data]
//
std::vector<char> ClientRequest::serialize(ClientRequest *request) {
    std::vector<char> buffer;
    
    // 1. Message Type
    append_to_buffer(buffer, request->type);
    
    // 2. Command Type
    append_to_buffer(buffer, request->command_type);

    // 3. File Name (the helper function adds the size first, then the data)
    append_to_buffer(buffer, request->file_name);

    // 4. File Data (add the size first, then the data)
    append_to_buffer(buffer, request->file_data);

    return buffer;
}

/**
 * Deserializes a byte vector into the ClientRequest object.
 */
void ClientRequest::deserialize(char* data, ClientRequest &request) {
    size_t offset = 0;

    // 1. Message Type (already known, but we skip it)
    offset += sizeof(MessageType);

    Command command = deserializeCommand(data + offset);

    request.command_type = command.type;
    request.file_name = command.file_name;
    request.file_data = command.data;
}

// ClientReply
ClientReply::ClientReply() {
    this->type = MessageType::CLIENT_REPLY;
}

ClientReply::ClientReply(Command cmd) :
    command(cmd)
{
    this->type = MessageType::CLIENT_REPLY;
}

/**
 * Serializes the ClientReply object into a byte vector.
 * Format: [type | status | message_size | message]
 */
std::vector<char> ClientReply::serialize(ClientReply *reply) {
    std::vector<char> buffer;

    // 1. Serialize message type
    append_to_buffer(buffer, reply->type);
    
    // 2. Serialize command
    // Allocate intermediate buff to serialize command into
    std::vector<char> command_buff = std::vector<char>(1024);
    int serialized_size = serializeCommand(reply->command, command_buff.data());

    // Create string to use append_to_buff
    std::string command_str(command_buff.data(), serialized_size);
    append_to_buffer(buffer, command_str);

    return buffer;
}

/**
 * Deserializes a byte vector into the ClientReply object.
 */
void ClientReply::deserialize(char* data, ClientReply &reply) {
    size_t offset = 0;

    // 1. Skip message type
    offset += sizeof(MessageType);

    // 2. Skip serialized command string length
    offset += sizeof(int);

    // 3. Deserialize command
    Command command = deserializeCommand(data + offset);

    reply.command = command;
}


// Forwarded request from primary to backup
class ForwardedRequest : public Message {
    
};

// Backup's acknowledge of request
class BackupReply : public Message {

};

ViewReply::ViewReply() {
    this->type = MessageType::VIEW_REPLY;
    this->view_num = 0;
    this->primary = "";
    this->backup = "";
}

ViewReply::ViewReply(int view_num, std::string primary, std::string backup)
    : view_num(view_num), primary(std::move(primary)), backup(std::move(backup)) {
    this->type = MessageType::VIEW_REPLY;
}

std::vector<char> ViewReply::serialize(ViewReply* reply) {
    std::vector<char> buffer;

    // 1. Message Type
    append_to_buffer(buffer, reply->type);

    // 2. View Number
    append_to_buffer(buffer, reply->view_num);

    // 3. Primary Server ID
    append_to_buffer(buffer, reply->primary);

    // 4. Backup Server ID
    append_to_buffer(buffer, reply->backup);

    return buffer;
}

void ViewReply::deserialize(char* data, ViewReply &reply) {
    size_t offset = 0;

    // 1. Message Type (skip)
    offset += sizeof(MessageType);

    // 2. View Number
    reply.view_num = read_int(data + offset);
    offset += sizeof(int);

    // 3. Primary Server ID
    int primary_len = read_int(data + offset);
    offset += sizeof(int);

    reply.primary = std::string(data + offset, primary_len);
    offset += primary_len;
    
    // 4. Backup Server ID
    int backup_len = read_int(data + offset);
    offset += sizeof(int);

    reply.backup = std::string(data + offset, backup_len);
}

Ping::Ping() {
    this->type = MessageType::PING;
}

Ping::Ping(int view_number, std::string id) : view_num(std::move(view_number)), server_id(std::move(id)) {
    this->type = MessageType::PING;
}

std::vector<char> Ping::serialize(Ping* p) {
    std::vector<char> buffer;
    
    // 1. Message Type
    append_to_buffer(buffer, p->type);

    // 2. Server's view #
    append_to_buffer(buffer, p->view_num);
    
    // 3. Server ID
    append_to_buffer(buffer, p->server_id);

    return buffer;
}

void Ping::deserialize(char* data, Ping &p) {
    size_t offset = 0;

    // 1. Message Type (skip)
    offset += sizeof(MessageType);
    
    // 2. View Number
    p.view_num = read_int(data + offset);
    // p.view_num.assign(data + offset, size_t(sizeof(int)));
    // std::memcpy(&(p.view_num), data + offset, sizeof(int));
    offset += sizeof(int);

    // // 3. Server ID
    int id_len;
    std::memcpy(&id_len, data + offset, sizeof(int));
    offset += sizeof(int);
    p.server_id = std::string(data + offset, id_len);

    std::cout << p.server_id << std::endl;
}