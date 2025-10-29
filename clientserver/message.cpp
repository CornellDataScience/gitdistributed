#include "message.hpp"
#include "commands.hpp"

class Message {
    
};

// Request from client to primary
class ClientRequest : public Message {
    int seqNum;
    Command cmd;
};

// Forwarded request from primary to backup
class ForwardedRequest : public Message {
    
};

// Backup's acknowledge of request
class BackupReply : public Message {
    int seqNum;
};

// Reply from primary to client
class ClientReply : public Message {

};