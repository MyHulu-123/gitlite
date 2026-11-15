#include "../include/Blob.h"
#include "../include/Utils.h"

Blob::Blob(const std::string& content){
    rawContent = content;
    bytes = std::to_string(rawContent.size());
    hash = Utils::sha1("blob " + bytes + "\0" + rawContent);
}
const std::string& Blob::getContent() const{
    return rawContent;
}
const std::string& Blob::getBytes() const{
    return bytes;
}
const std::string& Blob::getHash() const{
    return hash;
}
