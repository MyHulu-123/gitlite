#ifndef COMMIT_H
#define COMMIT_H
#include <string>
// 建议类：抽象一次提交（消息、时间戳、父提交、文件快照），可自由设计或删除。
// tag 是指向提交的仓库引用，不属于 Commit 对象自身。
class Commit {
private:
    std::string message;
    size_t time;
    std::string parent;
    std::string parent2;
    std::string tree;
    std::string hash;

public:
    Commit(const std::string& tree_, const size_t time_, const std::string& parent_, const std::string& message_, const std::string& parent2_);
    const std::string& getHash() const;
    const std::string& getTree() const;
    const size_t getTime() const;
    const std::string getTimeFormat() const;
    const std::string& getParent() const;
    const std::string& getParent2() const;
    const std::string& getMessage() const;
};
#endif // COMMIT_H
