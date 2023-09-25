#include <string>

class Parser
{
private:
    
    enum {
        S_BASE,
        S_FN_DECL,
        S_FN_DEF
    };

    std::string filename;

public:
    Parser();
    void Parse(std::string filename);
};