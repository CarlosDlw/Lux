#pragma once

#include <string>
#include <unordered_map>
#include <mutex>

// Tracks open documents in memory by URI.
class DocumentStore {
public:
    void open(const std::string& uri, const std::string& text);
    void update(const std::string& uri, const std::string& text);
    void close(const std::string& uri);

    // Returns empty string if not found.
    std::string get(const std::string& uri) const;
    bool contains(const std::string& uri) const;

    // Convert "file:///path/to/file.lx" → "/path/to/file.lx"
    static std::string uriToPath(const std::string& uri);

    // Convert "/path/to/file.lx" → "file:///path/to/file.lx"
    static std::string pathToUri(const std::string& path);

private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, std::string> docs_;
};
