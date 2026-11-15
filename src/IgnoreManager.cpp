#include "../include/IgnoreManager.h"
#include "../include/Utils.h"

IgnoreManager::IgnoreManager()
{
    if(Utils::isFile(".gitliteignore")){
        std::string content = Utils::readContentsAsString(".gitliteignore");
        std::stringstream ss(content);
        std::string line;
        while (std::getline(ss, line))
        {
            if(line.empty())continue;
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            if(line[0] == '#')continue;
            rule newrule;newrule.is_negation = false;
            size_t i = 0;
            if(line[0] == '!')newrule.is_negation = true, i++;
            if(i < line.length() && line[i] == '/'){
                i++;
            }
            for(;i < line.length(); i++){
                char c = line[i];
                if(c == '\\'){
                    if(i + 1 < line.length()){
                        newrule.words.push_back({line[i + 1], false});
                        i++;
                    }
                    else{
                        newrule.words.push_back({'\\', false});
                    }
                }
                else if(c == '*' || c == '?'){
                    newrule.words.push_back({c, true});
                }
                else newrule.words.push_back({c, false});
            }
            if(!newrule.words.empty())rules.push_back(newrule);
        }
        
    }
}

bool IgnoreManager::isMatch(const rule& _rule, const std::string& filename){
    int p = 0, s = 0;
    int star_idx = -1;
    int match_idx = 0;
    while (s < filename.length())
    {
        if(p < _rule.words.size() && !_rule.words[p].is_special && _rule.words[p].c == filename[s]){
            p++;s++;
        }
        else if(p < _rule.words.size() && _rule.words[p].is_special && _rule.words[p].c == '?'){
            p++;s++;
        }
        else if(p < _rule.words.size() && _rule.words[p].is_special && _rule.words[p].c == '*'){
            star_idx = p;
            match_idx = s;
            p++;
        }
        else if(star_idx != -1){
            p = star_idx + 1;
            match_idx++;
            s = match_idx;
        }
        else{
            return false;
        }
    }

    while (p < _rule.words.size() && _rule.words[p].is_special && _rule.words[p].c == '*')
    {
        p++;
    }
    return p == _rule.words.size();
}

bool IgnoreManager::isIgnore(const std::string& filename){
    bool ignored = false;
    for(const auto& rule : rules){
        if(isMatch(rule, filename)){
            ignored = !rule.is_negation;
        }
    }
    return ignored;
}