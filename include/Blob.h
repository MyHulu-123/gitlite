#ifndef BLOB_H
#define BLOB_H

#include <string>
class Blob {
private:
    std::string rawContent;
    std::string bytes;
    std::string hash;
public:
    Blob(const std::string& content);
    const std::string& getContent() const;
    const std::string& getHash() const;
    const std::string& getBytes() const;
    ~Blob() = default;
};

#endif // BLOB_H