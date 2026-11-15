#include "../include/ObjectDataBase.h"
#include "../include/Utils.h"
#include <regex>

void ObjectDataBase::writeBlob(const Blob& blob) {
    std::string filepath = Utils::join(".gitlite/objects", blob.getHash());
    Utils::writeContents(filepath, "blob\n" + blob.getContent());
}

void ObjectDataBase::writeTree(const Tree& tree) {
    std::string filepath = Utils::join(".gitlite/objects", tree.getHash());
    Utils::writeContents(filepath, "tree\n" + tree.getContent());
}

void ObjectDataBase::writeCommit(const Commit& commit) {
    std::string filepath = Utils::join(".gitlite/objects", commit.getHash());
    std::string content = "commit\n" + 
    commit.getTree() + "\n" + commit.getParent() + "\n" + 
    commit.getParent2() + "\n" + std::to_string(commit.getTime()) + "\n" + commit.getMessage();
    Utils::writeContents(filepath, content);
}

void ObjectDataBase::writeBranch(const std::string& name, const Commit& commit){
    std::string filepath = Utils::join(".gitlite/refs/heads", name);
    Utils::writeContents(filepath, commit.getHash());
}

void ObjectDataBase::writeBranch(const std::string& name, const std::string& commit_hash){
    std::string filepath = Utils::join(".gitlite/refs/heads", name);
    Utils::writeContents(filepath, commit_hash);
}

void ObjectDataBase::writeTag(const std::string& name, const Commit& commit){
    std::string filepath = Utils::join(".gitlite/refs/tags", name);
    Utils::writeContents(filepath, commit.getHash());
}

void ObjectDataBase::writeTag(const std::string& name, const std::string& commit_hash){
    std::string filepath = Utils::join(".gitlite/refs/tags", name);
    Utils::writeContents(filepath, commit_hash);
}

void ObjectDataBase::switchBranch(const std::string& branch){
    Utils::writeContents(".gitlite/HEADER", branch);
}

void ObjectDataBase::writeIndex(const Tree& tree){
    std::string filepath = ".gitlite/index";
    Utils::writeContents(filepath, tree.getContent());
}

Tree ObjectDataBase::readIndex(){
    std::string text = Utils::readContentsAsString(".gitlite/index");
    std::stringstream ss(text);
    std::string line;
    Tree index_;
    while(std::getline(ss, line)){
        std::stringstream lineString(line);
        std::string name,hash;
        lineString >> hash;
        char c;
        while (lineString.get(c) && c == ' ')
        {
            /* code */
        }
        if(lineString)lineString.unget();
        std::getline(lineString, name);
        index_.add(name, hash);
    }
    return index_;
}

Tree ObjectDataBase::readTree(const std::string& hash){
    std::string filepath = Utils::join(".gitlite/objects", hash);
    if(!Utils::isFile(filepath)){
        Tree tree;return tree;
    }
    std::string text = Utils::readContentsAsString(filepath);
    std::stringstream ss(text);
    std::string line;
    Tree tree;
    std::getline(ss, line);//读掉第一行
    while(std::getline(ss, line)){
        std::stringstream lineString(line);
        std::string name,hash;
        lineString >> hash;
        char c;
        while (lineString.get(c) && c == ' ')
        {
            /* code */
        }
        if(lineString)lineString.unget();
        std::getline(lineString, name);
        tree.add(name, hash);
    }
    return tree;
}

Blob ObjectDataBase::readBlob(const std::string& hash){
    std::string filepath = Utils::join(".gitlite/objects", hash);
    std::string content_head = Utils::readContentsAsString(filepath);
    size_t pos = content_head.find('\n');
    Blob blob(content_head.substr(pos+1));
    return blob;
}

Commit ObjectDataBase::readCommit(const std::string& hash){
    std::string filepath = Utils::join(".gitlite/objects", hash);
    std::string s[10];
    std::string text = Utils::readContentsAsString(filepath);
    std::stringstream ss(text);
    std::string line;
    size_t i = 0;
    std::getline(ss, line); //读掉第一行
    while (std::getline(ss, line))
    {
        s[i] = line; i++;
    }
    Commit commit(s[0],std::stoi(s[3]), s[1], s[4], s[2]);
    return commit;
}

Commit ObjectDataBase::readCurrentCommit(){
    std::string currentBranch = Utils::readContentsAsString(".gitlite/HEADER");
    std::string filepath = Utils::join(".gitlite/refs/heads", currentBranch);
    std::string hash = Utils::readContentsAsString(filepath);
    return readCommit(hash);
}

bool ObjectDataBase::isCommit(const std::string& hash){
    std::string content = Utils::readContentsAsString(".gitlite/objects/" + hash);
    size_t pos = content.find('\n');
    if(pos == std::string::npos)return false;
    std::string header = content.substr(0, pos);
    if(header.rfind("commit",0) == 0)return true;
    return false;
}

bool ObjectDataBase::isValidTagName(const std::string& tagname){
    std::regex pattern("^[A-Za-z0-9][A-Za-z0-9._-]*$");
    return std::regex_match(tagname, pattern);
}