#include "../include/Commit.h"
#include "../include/Utils.h"

Commit::Commit(const std::string& tree_, const size_t time_, const std::string& parent_, const std::string& message_, const std::string& parent2_) : 
tree{tree_},time{time_}, parent{parent_}, message{message_}, parent2{parent2_}
{
    hash = Utils::sha1("commit " + tree + "\0" + parent + "\0" + parent2 + "\0" + std::to_string(time) + "\0" + message);
}

const std::string& Commit::getHash() const{
    return hash;
}
const std::string& Commit::getTree() const{
    return tree;
}
const std::string& Commit::getParent() const{
    return parent;
}
const std::string& Commit::getParent2() const{
    return parent2;
}
const size_t Commit::getTime() const{
    return time;
}
const std::string& Commit::getMessage() const{
    return message;
}
const std::string Commit::getTimeFormat() const{
    time_t t = static_cast<time_t>(time);
    struct tm local_tm;
    localtime_r(&t, &local_tm);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y %z", &local_tm);
    return std::string(buffer);
}
// 建议类 Commit 的实现文件，可按需使用或删除。
