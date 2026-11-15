#include "../include/Tree.h"
#include "../include/Utils.h"

bool Tree::cmp(Entry x, Entry y){
    if(x.filename == y.filename)return x.blobHash < y.blobHash;
    return x.filename < y.filename;
}
const std::vector<Entry>& Tree::getEntries() const{
    return entries;
}
const std::string& Tree::getHash() const{
    return hash;
}
const std::string Tree::getContent() const{
    std::string s;
    for(auto& entry : entries){
        s += entry.blobHash + " " + entry.filename + "\n";
    }
    return s;
}
const std::string Tree::getBlobHash(const std::string& name) const{
    for(auto& entry : entries){
        if(entry.filename == name)return entry.blobHash;
    }
    return "";
}
bool Tree::exist(const std::string& name) const{
    for(auto& entry : entries){
        if(entry.filename == name)return true;
    }
    return false;
}
void Tree::updateHash(){
    std::string s = "tree ";
    for(auto& entry : entries){
        s += entry.filename + " " + entry.blobHash + "\0";
    }
    hash = Utils::sha1(s);
}
Tree::Tree(){
    updateHash();
}
void Tree::add(std::string filename, std::string blobHash){
    for(auto& entry : entries){
        if(entry.filename == filename){
            entry.blobHash = blobHash;
            return;
        }
    }
    entries.push_back({filename, blobHash});
    std::sort(entries.begin(), entries.end(), cmp);
    updateHash();
}
void Tree::remove(std::string filename){
    auto it = std::find_if(entries.begin(), entries.end(), 
        [&filename](const Entry& entry) {
            return entry.filename == filename;
        });
    if (it != entries.end()) {
        entries.erase(it);
    }
    updateHash();
}