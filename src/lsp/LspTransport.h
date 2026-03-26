#pragma once

#include <string>
#include <optional>

// JSON-RPC transport over stdin/stdout following the LSP base protocol.
// Messages use "Content-Length: N\r\n\r\n" framing.
class LspTransport {
public:
    // Block until a complete JSON message arrives on stdin.
    // Returns std::nullopt on EOF or read error.
    std::optional<std::string> readMessage();

    // Write a JSON message to stdout with proper Content-Length header.
    void sendMessage(const std::string& json);
};
