#include "lsp/DocumentStore.h"

void DocumentStore::open(const std::string& uri, const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    docs_[uri] = text;
}

void DocumentStore::update(const std::string& uri, const std::string& text) {
    std::lock_guard<std::mutex> lock(mutex_);
    docs_[uri] = text;
}

void DocumentStore::close(const std::string& uri) {
    std::lock_guard<std::mutex> lock(mutex_);
    docs_.erase(uri);
}

std::string DocumentStore::get(const std::string& uri) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = docs_.find(uri);
    if (it == docs_.end()) return {};
    return it->second;
}

bool DocumentStore::contains(const std::string& uri) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return docs_.count(uri) > 0;
}

std::string DocumentStore::uriToPath(const std::string& uri) {
    const std::string prefix = "file://";
    if (uri.rfind(prefix, 0) == 0) {
        return uri.substr(prefix.size());
    }
    return uri;
}

std::string DocumentStore::pathToUri(const std::string& path) {
    return "file://" + path;
}
