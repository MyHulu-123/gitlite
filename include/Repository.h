#ifndef REPOSITORY_H
#define REPOSITORY_H

#include "../include/ObjectDataBase.h"
#include <string>
#include <ctime>

// 建议类：抽象仓库的持久化状态，包括 HEAD、分支、标签、暂存区与对象库；
// 可自由扩充或重新设计。
// 注意：main.cpp 的 checkCWD 依赖 getGitliteDir()，请保留该方法。
class Repository {
private:
    void checkTagName(const std::string& tagname);
    void checkBranchName(const std::string& branchname);
    void checkNewBranchName(const std::string& branchname);
    void logCommit(const Commit& commit);
    std::string checkRevision(const std::string& revision);
    void checkoutFiles(const std::string& revision);
    void checkStagedFiles();
    Commit getParent(Commit& a, Commit& b);
public:
    // 元数据目录名，即 ".gitlite"
    static std::string getGitliteDir();
    void init();
    void add(const std::string& filename);
    void commit(const std::string& message);
    void rm(const std::string& filename);

    void log();
    void globalLog();
    void listTags();
    void tag(const std::string& tagname);
    void tag(const std::string& tagname, const std::string& revision);

    void status();
    void branch(const std::string& branchname);
    void rmBranch(const std::string& branchname);

    void checkoutBranch(const std::string& branchname);
    void checkoutFile(const std::string& filename);
    void checkoutFileInCommit(const std::string& revision, const std::string& filename);
    void reset(const std::string& revision);

    void merge(const std::string& branchname);
};

#endif // REPOSITORY_H;
