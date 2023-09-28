#include <string>

class Parser
{
private:
    
    enum State {
        S_BASE,
        S_MACRO,
        S_IDENTIFIER,
        S_FN_DECL,
        S_FN_DEF,
        S_COMMENT,
        S_COMMENT_LINE,
        S_COMMENT_BLOCK,
    };

    State state;
    unsigned int parenthesis_level;

public:
    Parser();
    void Parse(std::string filename);
    std::string ReadFile(std::string filename);
    bool isname(const char ch);
    bool nextline(const char* p);
};