#include "message.hpp"
#include <cstring>
#include <stdexcept>

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
    size_t str_len = str.length();
    append_to_buffer(buffer, str_len);
    buffer.insert(buffer.end(), str.begin(), str.end());
}

// --- Message ---
MessageType Message::peek_type(const std::vector<char>& data) {
    if (data.size() < sizeof(MessageType)) {
        throw std::runtime_error("Buffer too small to peek message type");
    }
    MessageType type;
    std::memcpy(&type, data.data(), sizeof(MessageType));
    return type;
}


// --- ClientRequest ---
ClientRequest::ClientRequest() {
    this->type = MessageType::CLIENT_REQUEST;
}
/**
ClientRequest::ClientRequest(CommandType cmd, std::string name, std::vector<char> data)
    : type(MessageType::CLIENT_REQUEST),
      command_type(cmd),
      file_name(std::move(name)),
      file_data(std::move(data)) */
ClientRequest::ClientRequest(CommandType cmd) :
    command_type(cmd),
    file_name(""),
    file_data({})
{
    this->type = MessageType::CLIENT_REQUEST;
}

ClientRequest::ClientRequest(CommandType cmd, std::string name, std::vector<char> data) {
    this->type = MessageType::CLIENT_REQUEST;
    this->command_type = cmd;
    this->file_name = std::move(name);
    this->file_data = std::move(data);
}

//
// Serializes the ClientRequest object into a byte vector.
// Format: [type | commandType | file_name_size | file_name | file_data_size | file_data]
//
std::vector<char> ClientRequest::serialize() {
    std::vector<char> buffer;
    
    // 1. Message Type
    append_to_buffer(buffer, this->type);
    
    // 2. Command Type
    append_to_buffer(buffer, this->command_type);

    // 3. File Name (the helper function adds the size first, then the data)
    append_to_buffer(buffer, this->file_name);

    // 4. File Data (add the size first, then the data)
    size_t file_data_len = this->file_data.size();
    append_to_buffer(buffer, file_data_len);
    buffer.insert(buffer.end(), this->file_data.begin(), this->file_data.end());

    return buffer;
}

/**
 * Deserializes a byte vector into the ClientRequest object.
 */
void ClientRequest::deserialize(const std::vector<char>& data) {
    size_t offset = 0;

    // 1. Message Type (already known, but we skip it)
    offset += sizeof(MessageType);

    // 2. Command Type
    std::memcpy(&this->command_type, data.data() + offset, sizeof(CommandType));
    offset += sizeof(CommandType);

    // 3. File Name
    size_t file_name_len;
    std::memcpy(&file_name_len, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    this->file_name.assign(data.data() + offset, file_name_len);
    offset += file_name_len;

    // 4. File Data
    size_t file_data_len;
    std::memcpy(&file_data_len, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    this->file_data.assign(data.data() + offset, data.data() + offset + file_data_len);
}

// ClientReply
ClientReply::ClientReply() {
    this->type = MessageType::CLIENT_REPLY;
}

ClientReply::ClientReply(std::string message) : reply_message(std::move(message)) {
    this->type = MessageType::CLIENT_REPLY;
}

/**
 * Serializes the ClientReply object into a byte vector.
 * Format: [type | status | message_size | message]
 */
std::vector<char> ClientReply::serialize() {
    std::vector<char> buffer;
    
    // 1. Message Type
    append_to_buffer(buffer, this->type);
    
    // 2. Status
    append_to_buffer(buffer, this->status);

    // 3. Reply Message
    append_to_buffer(buffer, this->reply_message);

    return buffer;
}

/**
 * Deserializes a byte vector into the ClientReply object.
 */
void ClientReply::deserialize(const std::vector<char>& data) {
    size_t offset = 0;

    // 1. Message Type (skip)
    offset += sizeof(MessageType);

    // 2. Status
    std::memcpy(&this->status, data.data() + offset, sizeof(int));
    offset += sizeof(int);

    // 3. Reply Message
    size_t msg_len;
    std::memcpy(&msg_len, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    this->reply_message.assign(data.data() + offset, msg_len);
}


// Forwarded request from primary to backup
class ForwardedRequest : public Message {
    
};

// Backup's acknowledge of request
class BackupReply : public Message {

};

ViewReply::ViewReply() {
    this->type = MessageType::PRIMARY_REPLY;
    this->view_num = 0;
    this->primary = "";
    this->backup = "";
}

ViewReply::ViewReply(int view_num, std::string primary, std::string backup)
    : view_num(view_num), primary(std::move(primary)), backup(std::move(backup)) {
    this->type = MessageType::PRIMARY_REPLY;
}

std::vector<char> ViewReply::serialize() {
    std::vector<char> buffer;

    // 1. Message Type
    append_to_buffer(buffer, this->type);

    // 2. View Number
    append_to_buffer(buffer, this->view_num);

    // 3. Primary Server ID
    append_to_buffer(buffer, this->primary);

    // 4. Backup Server ID
    append_to_buffer(buffer, this->backup);

    return buffer;
}

void ViewReply::deserialize(const std::vector<char>& data) {
    size_t offset = 0;

    // 1. Message Type (skip)
    offset += sizeof(MessageType);

    // 2. View Number
    std::memcpy(&this->view_num, data.data() + offset, sizeof(int));
    offset += sizeof(int);

    // 3. Primary Server ID
    size_t primary_len;
    std::memcpy(&primary_len, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    this->primary.assign(data.data() + offset, primary_len);
    offset += primary_len;

    // 4. Backup Server ID
    size_t backup_len;
    std::memcpy(&backup_len, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    this->backup.assign(data.data() + offset, backup_len);
}

Ping::Ping() {
    this->type = MessageType::PING;
}

Ping::Ping(int view_number, std::string id) : view_num(std::move(view_number)), server_id(std::move(id)) {
    this->type = MessageType::PING;
}

std::vector<char> Ping::serialize() {
    std::vector<char> buffer;
    
    // 1. Message Type
    append_to_buffer(buffer, this->type);

    // 2. Server's view #
    append_to_buffer(buffer, this->view_num);
    
    // 3. Server ID
    append_to_buffer(buffer, this->server_id);

    return buffer;
}

void Ping::deserialize(const std::vector<char>& data) {
    size_t offset = 0;

    // 1. Message Type (skip)
    offset += sizeof(MessageType);
    
    // 2. View Number
    std::memcpy(&this->view_num, data.data() + offset, sizeof(int));
    offset += sizeof(int);

    // 3. Server ID
    size_t id_len;
    std::memcpy(&id_len, data.data() + offset, sizeof(size_t));
    offset += sizeof(size_t);
    this->server_id.assign(data.data() + offset, id_len);
}