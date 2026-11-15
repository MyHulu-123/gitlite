#ifndef OBJECT_DATA_BASE_H
#define OBJECT_DATA_BASE_H
#include <string>
#include "../include/Blob.h"
#include "../include/Commit.h"
#include "../include/Tree.h"
class ObjectDataBase {//专门处理文件读取和写入，并转成对应的类
private:
    /* data */
public:
    //ObjectDataBase();
    static void writeBlob(const Blob& blob);
    static void writeTree(const Tree& tree);
    static void writeCommit(const Commit& commit);

    static void writeBranch(const std::string& name, const Commit& commit);
    static void writeBranch(const std::string& name, const std::string& commit_hash);

    static void writeTag(const std::string& name, const Commit& commit);
    static void writeTag(const std::string& name, const std::string& commit_hash);

    static void switchBranch(const std::string& branch);

    static Blob readBlob(const std::string& hash);
    static Tree readTree(const std::string& hash);
    static Commit readCommit(const std::string& hash);

    static void writeIndex(const Tree& tree);
    static Tree readIndex();

    static Commit readCurrentCommit();

    static bool isCommit(const std::string& hash);
    static bool isValidTagName(const std::string& tagname);
};

#endif // OBJECT_DATA_BASE_H