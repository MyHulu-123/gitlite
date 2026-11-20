#include "../include/Repository.h"
#include "../include/IgnoreManager.h"
#include "../include/Utils.h"
#include <set>
#include <queue>
// main.cpp 的 checkCWD 依赖此方法，请保留。
std::string Repository::getGitliteDir() {
    return ".gitlite";
}

void Repository::init(){
    if(Utils::isDirectory(".gitlite")){
        Utils::exitWithMessage("A Gitlite version-control system already exists in the current directory.");
    }
    Utils::createDirectories(".gitlite");
    Utils::createDirectories(".gitlite/objects");
    Utils::createDirectories(".gitlite/refs");
    Utils::createDirectories(".gitlite/refs/heads");
    Utils::createDirectories(".gitlite/refs/tags");
    Commit commit("",0,"","initial commit","");
    ObjectDataBase::writeCommit(commit);
    ObjectDataBase::writeBranch("master", commit);
    ObjectDataBase::switchBranch("master");
    Tree tree;
    ObjectDataBase::writeIndex(tree);
}

void Repository::add(const std::string& filename){
    if(!Utils::exists(filename)){
        Utils::exitWithMessage("File does not exist.");
    }
    std::string content = Utils::readContentsAsString(filename);
    Blob blob(content);
    Tree index_ = ObjectDataBase::readIndex();
    //比较文件与前一次提交
    
    Commit currentCommit = ObjectDataBase::readCurrentCommit();
    Tree currentTree = ObjectDataBase::readTree(currentCommit.getTree());
    /*
    if(currentTree.exist(filename) && currentTree.getBlobHash(filename) == blob.getHash()){
        index_.remove(filename);
        return;
    }
    */
    bool isTracked = currentTree.exist(filename) || index_.exist(filename);
    if(!isTracked){
        IgnoreManager ignoreManager;
        if(ignoreManager.isIgnore(filename))return;
    }
    if(!Utils::exists(".gitlite/objects/" + blob.getHash())){
        ObjectDataBase::writeBlob(blob);
    }
    
    index_.add(filename, blob.getHash());
    ObjectDataBase::writeIndex(index_);
}

void Repository::commit(const std::string& message){
    if(message.empty())Utils::exitWithMessage("Please enter a commit message.");
    Commit currentCommit = ObjectDataBase::readCurrentCommit();
    Tree currentTree = ObjectDataBase::readTree(currentCommit.getTree());
    Tree index_ = ObjectDataBase::readIndex();

    if(currentTree.getHash() == index_.getHash()){
        Utils::exitWithMessage("No changes added to the commit.");
    }

    ObjectDataBase::writeTree(index_);
    time_t now = time(nullptr);
    Commit commit(index_.getHash(), now, currentCommit.getHash(), message,"");

    ObjectDataBase::writeCommit(commit);
    std::string currentBranch = Utils::readContentsAsString(".gitlite/HEADER");
    ObjectDataBase::writeBranch(currentBranch, commit);
}

void Repository::rm(const std::string& filename){
    Commit currentCommit = ObjectDataBase::readCurrentCommit();
    Tree currentTree = ObjectDataBase::readTree(currentCommit.getTree());
    Tree index_ = ObjectDataBase::readIndex();

    if(currentTree.exist(filename)){
        index_.remove(filename);
        ObjectDataBase::writeIndex(index_);
        Utils::restrictedDelete(filename);
        return;
    }
    else{
        if(index_.exist(filename)){
            index_.remove(filename);
            ObjectDataBase::writeIndex(index_);
            return;
        }
        Utils::exitWithMessage("No reason to remove the file.");
    }
}

void Repository::logCommit(const Commit& commit){
    Utils::message("===");
    Utils::message("commit " + commit.getHash());
    if(!commit.getParent2().empty()){
        Utils::message("Merge: "+commit.getParent().substr(0,7) + " " + commit.getParent2().substr(0,7));
    }
    Utils::message("Date: " + commit.getTimeFormat());
    Utils::message(commit.getMessage());
    Utils::message("");
}

void Repository::log(){
    Commit currentCommit = ObjectDataBase::readCurrentCommit();
    while (true)
    {
        logCommit(currentCommit);
        if(currentCommit.getParent().empty())break;
        currentCommit = ObjectDataBase::readCommit(currentCommit.getParent());
    }
}

void Repository::globalLog(){
    std::set<std::string> hashSet;
    std::vector<Commit> commitPool;
    std::queue<Commit> q;

    std::vector<std::string> branches = Utils::plainFilenamesIn(".gitlite/refs/heads");
    for(const auto& branch : branches){
        std::string commitHash = Utils::readContentsAsString(".gitlite/refs/heads/" + branch);
        if(hashSet.find(commitHash) == hashSet.end()){//不存在，将它加入
            hashSet.insert(commitHash);
            Commit commit = ObjectDataBase::readCommit(commitHash);
            commitPool.push_back(commit);
            q.push(commit);
        }
    }

    std::vector<std::string> tags = Utils::plainFilenamesIn(".gitlite/refs/tags");
    for(const auto& tag : tags){
        std::string commitHash = Utils::readContentsAsString(".gitlite/refs/tags/" + tag);
        if(hashSet.find(commitHash) == hashSet.end()){//不存在，将它加入
            hashSet.insert(commitHash);
            Commit commit = ObjectDataBase::readCommit(commitHash);
            commitPool.push_back(commit);
            q.push(commit);
        }
    }

    while (!q.empty())
    {
        Commit commit = q.front();q.pop();
        if(!commit.getParent().empty()){
            std::string parent = commit.getParent();
            if(hashSet.find(parent) == hashSet.end()){
                hashSet.insert(parent);
                Commit nxt = ObjectDataBase::readCommit(parent);
                commitPool.push_back(nxt);
                q.push(nxt);
            }
        }
        if(!commit.getParent2().empty()){
            std::string parent2 = commit.getParent2();
            if(hashSet.find(parent2) == hashSet.end()){
                hashSet.insert(parent2);
                Commit nxt = ObjectDataBase::readCommit(parent2);
                commitPool.push_back(nxt);
                q.push(nxt);
            }
        }
    }
    
    std::sort(commitPool.begin(), commitPool.end(), [](const Commit& a, const Commit& b){
        if(a.getTime() == b.getTime()){
            return a.getHash() < b.getHash();
        }
        return a.getTime() > b.getTime();
    });
    
    for(const auto& commit : commitPool){
        logCommit(commit);
    }
}

void Repository::listTags(){
    std::vector<std::string> tags = Utils::plainFilenamesIn(".gitlite/refs/tags");
    std::sort(tags.begin(), tags.end());
    std::string filepath;
    for(const auto& tag : tags){
        filepath = Utils::join(".gitlite/refs/tags",tag);
        Utils::message(tag + " " + Utils::readContentsAsString(filepath));
    }
}

void Repository::checkTagName(const std::string& tagname){
    if(!ObjectDataBase::isValidTagName(tagname)){
        Utils::exitWithMessage("Incorrect operands.");
    }
    if(Utils::isFile(".gitlite/refs/tags/" + tagname)){
        Utils::exitWithMessage("A tag with that name already exists.");
    }
}

void Repository::tag(const std::string& tagname){
    checkTagName(tagname);
    Commit currentCommit = ObjectDataBase::readCurrentCommit();
    ObjectDataBase::writeTag(tagname, currentCommit);
}

std::string Repository::checkRevision(const std::string& revision){
    if(Utils::isFile(".gitlite/refs/tags/" + revision)){//tag
        return Utils::readContentsAsString(".gitlite/refs/tags/" + revision);
    }
    std::vector<std::string> hashes = Utils::plainFilenamesIn(".gitlite/objects");
    std::sort(hashes.begin(), hashes.end());
    for(const auto& hash : hashes){
        if(hash.rfind(revision, 0) == 0 && ObjectDataBase::isCommit(hash)){
            return hash;
        }
    }
    Utils::exitWithMessage("No commit with that id exists.");
    return "";
}

void Repository::tag(const std::string& tagname, const std::string& revision){
    checkTagName(tagname);
    std::string commitHash = checkRevision(revision);
    ObjectDataBase::writeTag(tagname, commitHash);
}

void Repository::status(){
    std::set<std::string> allfiles;
    Tree head = ObjectDataBase::readTree(ObjectDataBase::readCurrentCommit().getTree());
    Tree idx = ObjectDataBase::readIndex();
    std::vector<std::string> wtfiles = Utils::plainFilenamesIn(".");
    for(const auto& entry : head.getEntries())allfiles.insert(entry.filename);
    for(const auto& entry : idx.getEntries())allfiles.insert(entry.filename);
    IgnoreManager ignoreManager;
    for(const auto& filename : wtfiles){
        if(ignoreManager.isIgnore(filename))continue;
        else allfiles.insert(filename);
    }
    std::vector<std::string> stagedFiles;
    std::vector<std::string> removedFiles;
    std::vector<std::string> notStagedFiles;
    std::vector<std::string> untrackedFiles;

    for(const auto& f : allfiles){
        bool inHead = head.exist(f);
        bool inIndex = idx.exist(f);
        bool inWt = Utils::isFile(f);

        std::string headHash = inHead ? head.getBlobHash(f) : "";
        std::string indexHash = inIndex ? idx.getBlobHash(f) : "";
        std::string wtHash = "";
        if(inWt){
            Blob temp(Utils::readContentsAsString(f));
            wtHash = temp.getHash();
        }

        if(inIndex && (!inHead || headHash != indexHash)){
            stagedFiles.push_back(f);
        }
        if(inHead && !inIndex){
            removedFiles.push_back(f);
        }
        if(!inIndex && inWt){
            untrackedFiles.push_back(f);
        }
        //不能使用if-else语句，文件可能既是被删除了，也是未跟着
        if(inIndex){
            if(!inWt){
                notStagedFiles.push_back(f + " (deleted)");
            }
            else if(indexHash != wtHash){
                notStagedFiles.push_back(f + " (modified)");
            }
        }

    }

    Utils::message("=== Branches ===");
    std::vector<std::string> branches = Utils::plainFilenamesIn(".gitlite/refs/heads");
    std::string currentBranch = Utils::readContentsAsString(".gitlite/HEADER");
    for(const auto& b : branches){
        if(b == currentBranch)Utils::message("*" + b);
        else Utils::message(b);
    }
    Utils::message("");

    Utils::message("=== Staged Files ===");
    for(const auto& f : stagedFiles){
        Utils::message(f);
    }
    Utils::message("");

    Utils::message("=== Removed Files ===");
    for(const auto& f : removedFiles){
        Utils::message(f);
    }
    Utils::message("");

    Utils::message("=== Modifications Not Staged For Commit ===");
    for(const auto& f : notStagedFiles){
        Utils::message(f);
    }
    Utils::message("");

    Utils::message("=== Untracked Files ===");
    for(const auto& f : untrackedFiles){
        Utils::message(f);
    }
}

void Repository::checkBranchName(const std::string& branchname){
    std::string filepath = ".gitlite/refs/heads/" + branchname;
    if(!Utils::isFile(filepath)){
        Utils::exitWithMessage("A branch with that name does not exist.");
    }
}

void Repository::checkNewBranchName(const std::string& branchname){
    if(!ObjectDataBase::isValidTagName(branchname)){
        Utils::exitWithMessage("Incorrect operands.");
    }
    if(Utils::isFile(".gitlite/refs/heads/" + branchname)){
        Utils::exitWithMessage("A branch with that name already exists.");
    }
}

void Repository::branch(const std::string& branchname){
    checkNewBranchName(branchname);
    Commit currentCommit = ObjectDataBase::readCurrentCommit();
    ObjectDataBase::writeBranch(branchname, currentCommit);
}

void Repository::rmBranch(const std::string& branchname){
    std::string filepath = ".gitlite/refs/heads/" + branchname;
    checkBranchName(branchname);
    if(branchname == Utils::readContentsAsString(".gitlite/HEADER")){
        Utils::exitWithMessage("Cannot remove the current branch.");
    }
    remove(filepath.c_str());
}

void Repository::checkoutBranch(const std::string& branchname){
    std::string filepath = ".gitlite/refs/heads/" + branchname;
    if(!Utils::isFile(filepath)){
        Utils::exitWithMessage("No such branch exists.");
    }
    if(branchname == Utils::readContentsAsString(".gitlite/HEADER")){
        Utils::exitWithMessage("No need to checkout the current branch.");
    }
    checkoutFiles(Utils::readContentsAsString(".gitlite/refs/heads/" + branchname));
    ObjectDataBase::switchBranch(branchname);
}

void Repository::checkoutFile(const std::string& filename){
    checkoutFileInCommit(ObjectDataBase::readCurrentCommit().getHash(), filename);
}

void Repository::checkoutFileInCommit(const std::string& revision, const std::string& filename){
    std::string commitHash = checkRevision(revision);
    Commit commit = ObjectDataBase::readCommit(commitHash);
    Tree tree = ObjectDataBase::readTree(commit.getTree());
    if(!tree.exist(filename)){
        Utils::exitWithMessage("File does not exist in that commit.");
    }
    Blob temp = ObjectDataBase::readBlob(tree.getBlobHash(filename));
    Utils::writeContents(filename, temp.getContent());
}

void Repository::reset(const std::string& revision){
    std::string commitHash = checkRevision(revision);
    checkoutFiles(commitHash);
    ObjectDataBase::writeBranch(Utils::readContentsAsString(".gitlite/HEADER"), commitHash);
}

void Repository::checkoutFiles(const std::string& revision){//通过目标Commit修改工作区和暂存区
    
    std::string commitHash = checkRevision(revision);
    Tree head = ObjectDataBase::readTree(ObjectDataBase::readCurrentCommit().getTree());
    Tree idx = ObjectDataBase::readIndex();
    Commit newTarget = ObjectDataBase::readCommit(commitHash);
    Tree newTree = ObjectDataBase::readTree(newTarget.getTree());
    std::set<std::string> allfiles;

    std::vector<std::string> wtfiles = Utils::plainFilenamesIn(".");
    IgnoreManager ignoreManager;
    for(const auto& filename : wtfiles){
        if(ignoreManager.isIgnore(filename))continue;
        else allfiles.insert(filename);
    }

    for(const auto& f : allfiles){
        bool inHead = head.exist(f);
        bool inIndex = idx.exist(f);
        bool inNewTree = newTree.exist(f);
        bool inWt = Utils::isFile(f);
        if(inNewTree && !inHead && !inIndex && inWt){
            Utils::exitWithMessage("There is an untracked file in the way; delete it, or add and commit it first.");
        }
    }
    allfiles.clear();
    for(const auto& entry : head.getEntries()){
        if(!newTree.exist(entry.filename) ){
            Utils::restrictedDelete(entry.filename);
        }
    }
    for (const auto& entry : newTree.getEntries()) {
        Blob b = ObjectDataBase::readBlob(entry.blobHash);
        Utils::writeContents(entry.filename, b.getContent()); 
    }
    ObjectDataBase::writeIndex(newTree);
}

//merge
void Repository::checkStagedFiles(){
    Tree idx = ObjectDataBase::readIndex();
    Tree head = ObjectDataBase::readTree(ObjectDataBase::readCurrentCommit().getTree());
    if(idx.getHash() != head.getHash()){
        Utils::exitWithMessage("You have uncommitted changes.");
    }
}

Commit Repository::getParent(Commit& currentHead, Commit& branchHead){
    std::set<Commit> currentParents;
    std::queue<Commit> c_parents;
    c_parents.push(currentHead);currentParents.insert(currentHead);
    while (!c_parents.empty())
    {
        Commit _p = c_parents.front();c_parents.pop();
        if(!_p.getParent().empty()){
            Commit _p1 = ObjectDataBase::readCommit(_p.getParent());
            if(currentParents.find(_p1) == currentParents.end())c_parents.push(_p1), currentParents.insert(_p);
        }
        if(!_p.getParent2().empty()){
            Commit _p2 = ObjectDataBase::readCommit(_p.getParent2());
            if(currentParents.find(_p2) == currentParents.end())c_parents.push(_p2), currentParents.insert(_p);
        }
    }

    std::queue<Commit> b_parents;
    std::set<Commit> triedParents;
    b_parents.push(branchHead);
    triedParents.insert(branchHead);
    while (!b_parents.empty())
    {
        Commit _p = b_parents.front();b_parents.pop();
        if(currentParents.find(_p) != currentParents.end()){
            return _p;
        }
        if(!_p.getParent().empty()){
            Commit _p1 = ObjectDataBase::readCommit(_p.getParent());
            if(triedParents.find(_p1) == triedParents.end())b_parents.push(_p1), triedParents.insert(_p);
        }
        if(!_p.getParent2().empty()){
            Commit _p2 = ObjectDataBase::readCommit(_p.getParent2());
            if(triedParents.find(_p2) == triedParents.end())b_parents.push(_p2), triedParents.insert(_p);
        }
    }
    return Commit("",0,"","wrong, you shouldn't see this.","");
    
}

enum class MergeAction{
    KEEP_OURS,
    WRITE_THEIRS,
    DELETE_FILE,
    CONFLICT
};


void Repository::merge(const std::string& branchname){
    checkStagedFiles();
    checkBranchName(branchname);
    if(Utils::readContentsAsString(".gitlite/HEADER") == branchname){
        Utils::exitWithMessage("Cannot merge a branch with itself.");
    }
    std::string currentBranch = Utils::readContentsAsString(".gitlite/HEADER");
    Commit currentHead = ObjectDataBase::readCurrentCommit();
    Commit branchHead = ObjectDataBase::readCommit(Utils::readContentsAsString(".gitlite/refs/heads/" + branchname));
    Commit parent = getParent(currentHead, branchHead);
    
    if(parent.getHash() == branchHead.getHash()){
        Utils::exitWithMessage("Given branch is an ancestor of the current branch.");
    }
    if(parent.getHash() == currentHead.getHash()){//fast-forward
        checkoutFiles(branchHead.getHash());
        ObjectDataBase::writeBranch(currentBranch, branchHead);
        Utils::exitWithMessage("Current branch fast-forwarded.");
    }
    //目前，index和currentHead一致
    std::map<std::string, MergeAction> planned;
    std::set<std::string> allfiles;
    Tree currentTree = ObjectDataBase::readTree(currentHead.getTree());
    Tree branchTree = ObjectDataBase::readTree(branchHead.getTree());
    Tree parentTree = ObjectDataBase::readTree(parent.getTree());
    for(const auto& entry : currentTree.getEntries())allfiles.insert(entry.filename);
    for(const auto& entry : branchTree.getEntries())allfiles.insert(entry.filename);
    for(const auto& entry : parentTree.getEntries())allfiles.insert(entry.filename);

    for(auto& f : allfiles){
        bool inParent = parentTree.exist(f);
        bool inCurrent = currentTree.exist(f);
        bool inBranch = branchTree.exist(f);
        std::string hashParent = parentTree.getBlobHash(f);
        std::string hashCurrent = currentTree.getBlobHash(f);
        std::string hashBranch = branchTree.getBlobHash(f);

        if(inParent && inCurrent && inBranch && hashParent == hashCurrent && hashParent != hashBranch){
            planned[f] = MergeAction::WRITE_THEIRS;
        }
        else if(inParent && inCurrent && inBranch && hashParent != hashCurrent && hashParent == hashBranch)continue;
        else if(hashCurrent == hashBranch)continue;
        else if(!inParent && !inBranch && inCurrent)continue;
        else if(!inParent && inBranch && !inCurrent)planned[f] = MergeAction::WRITE_THEIRS;
        else if(inParent && hashParent == hashCurrent && !inBranch)planned[f] = MergeAction::DELETE_FILE;
        else if(inParent && hashParent == hashBranch && !inCurrent)continue;
        else if(hashBranch != hashCurrent)planned[f] = MergeAction::CONFLICT;
    }


    std::vector<std::string> wtfiles = Utils::plainFilenamesIn(".");
    std::set<std::string> trueWtfiles;
    IgnoreManager ignoreManager;
    for(const auto& filename : wtfiles){
        if(ignoreManager.isIgnore(filename))continue;
        else trueWtfiles.insert(filename);
    }

    for(const auto& f : trueWtfiles){
        if(planned.find(f) != planned.end() && !currentTree.exist(f)){
            Utils::exitWithMessage("There is an untracked file in the way; delete it, or add and commit it first.");
        }
    }
    Tree idx = ObjectDataBase::readIndex();
    bool haveConflict = false;
    for(const auto& f : planned){
        if (f.second == MergeAction::WRITE_THEIRS){
            Blob b = ObjectDataBase::readBlob(branchTree.getBlobHash(f.first));
            Utils::writeContents(f.first, b.getContent());
            idx.add(f.first, b.getHash());
        }
        else if(f.second == MergeAction::DELETE_FILE){
            Utils::restrictedDelete(f.first);
            idx.remove(f.first);
        }
        else if(f.second == MergeAction::CONFLICT){
            Blob currentBlob = Blob("");
            if(!currentTree.getBlobHash(f.first).empty())
            currentBlob = ObjectDataBase::readBlob(currentTree.getBlobHash(f.first));
            Blob branchBlob = Blob("");
            if(!branchTree.getBlobHash(f.first).empty())
            branchBlob = ObjectDataBase::readBlob(branchTree.getBlobHash(f.first));
            std::string currentContent = currentBlob.getContent();
            std::string branchContent = branchBlob.getContent();
            if(!currentContent.empty() && currentContent.back() != '\n')currentContent += '\n';
            if(!branchContent.empty() && branchContent.back() != '\n')branchContent += '\n';
            std::string final = "<<<<<<< HEAD\n" + currentContent + "=======\n" + branchContent + ">>>>>>>\n";
            Blob finalBlob(final);
            ObjectDataBase::writeBlob(finalBlob);
            Utils::writeContents(f.first, final);
            idx.add(f.first, finalBlob.getHash());

            haveConflict = true;
        }
    }

    //合并提交
    ObjectDataBase::writeIndex(idx);
    ObjectDataBase::writeTree(idx);
    std::string message = "Merged " + branchname + " into " + currentBranch + ".";
    time_t now = time(nullptr);
    Commit commitMerge(idx.getHash(), now, currentHead.getHash(), message, branchHead.getHash());
    ObjectDataBase::writeCommit(commitMerge);
    if(haveConflict)Utils::message("Encountered a merge conflict.");
    ObjectDataBase::writeBranch(currentBranch, commitMerge);
}