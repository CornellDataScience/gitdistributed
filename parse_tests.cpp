
#include "parse.cpp"
#include "server.cpp"

void test_round_trip(const Message &original)
{
    std::vector<char> buf = serialize(original);
    Message reconstructed = deserialize(buf.data());

    assert(reconstructed.type == original.type);
    assert(reconstructed.file_name == original.file_name);
    assert(reconstructed.data == original.data);

    std::cout << "Round-trip test passed for type " << static_cast<int>(original.type) << "\n";
}


void test_client_push_basic() {
    Message msg{ MessageType::CLIENT_PUSH, "test.txt", "hello world" };
    test_round_trip(msg);
}

void test_server_pull_basic() {
    Message msg{ MessageType::SERVER_PULL, "file1.bin", "12345" };
    test_round_trip(msg);
}

void test_empty_fields() {
    Message msg{ MessageType::CLIENT_PUSH, "", "" };
    test_round_trip(msg);
}

void test_client_pull_no_payload() {
    Message msg{ MessageType::CLIENT_PULL, "", "" };
    test_round_trip(msg);
}

void test_server_push_no_payload() {
    Message msg{ MessageType::SERVER_PUSH, "", "" };
    test_round_trip(msg);
}

int main() {
    test_client_push_basic();
    test_server_pull_basic();
    test_empty_fields();
    test_client_pull_no_payload();
    test_server_push_no_payload();
    std::cout << "🎉 All round-trip serialization tests passed.\n";
    return 0;
}
