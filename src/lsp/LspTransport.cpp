#include "lsp/LspTransport.h"

#include <iostream>
#include <sstream>
#include <cstdlib>

std::optional<std::string> LspTransport::readMessage() {
    // Read headers until we find "Content-Length: <n>"
    int contentLength = -1;

    while (true) {
        std::string line;
        if (!std::getline(std::cin, line)) {
            return std::nullopt; // EOF
        }

        // Strip trailing \r if present (LSP uses \r\n)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Empty line = end of headers
        if (line.empty()) {
            break;
        }

        // Parse Content-Length header
        const std::string prefix = "Content-Length: ";
        if (line.rfind(prefix, 0) == 0) {
            contentLength = std::atoi(line.c_str() + prefix.size());
        }
        // Ignore other headers (Content-Type, etc.)
    }

    if (contentLength <= 0) {
        return std::nullopt;
    }

    // Read exactly contentLength bytes
    std::string body(contentLength, '\0');
    if (!std::cin.read(&body[0], contentLength)) {
        return std::nullopt;
    }

    return body;
}

void LspTransport::sendMessage(const std::string& json) {
    std::cout << "Content-Length: " << json.size() << "\r\n"
              << "\r\n"
              << json;
    std::cout.flush();
}
