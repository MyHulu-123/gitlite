#ifndef TREE_H
#define TREE_H
#include <string>
#include <vector>
#include <map>
#include <algorithm>
struct Entry
{
    std::string filename;
    std::string blobHash;
};

class Tree {
private:
    std::vector<Entry> entries;
    std::string hash;
    static bool cmp(Entry x,Entry y);
    void updateHash();
public:
    Tree();
    const std::vector<Entry>& getEntries() const;
    const std::string& getHash() const;
    const std::string getContent() const;
    const std::string getBlobHash(const std::string& name) const;
    bool exist(const std::string& name) const;
    void add(std::string filename, std::string blobHash);
    void remove(std::string filename);
};

#endif // TREE_H