#include <vector>
#include <string>
struct word
{
    char c;
    bool is_special;
};

struct rule
{
    std::vector<word> words;
    bool is_negation;     //是否反选
};
class IgnoreManager
{
private:
    std::vector<rule> rules;  //规则集
    bool isMatch(const rule& _rule, const std::string& filename);
public:
    IgnoreManager();
    bool isIgnore(const std::string& filename);
};

