#include <string>
#include <stack>
#include <vector>
#include "construct.h"

typedef Construct Ct;

class Parser
{
private:
    
    enum class PExcept {
        E_EOF,
        E_UNRESOLVED,
    };

    const char* file_buff;
    std::vector<std::string> tokens;
    std::stack<Ct> level;
    std::vector<std::string> functions;
    std::string token;
    unsigned int parenthesis_level;

    Ct& ct_top() { return level.top(); }
    bool ct_top_is(Ct::Type type) { return type == level.top().type(); }
    void ct_push(Ct construct);
    void ct_push(Ct::Type type);
    void ct_pop();

public:
    Parser();
    void Parse(std::string filename);
    char* ReadFile(std::string filename);
    bool isspecifier(const char ch);
    bool isalnum_us(const char ch);
    bool isalpha_us(const char ch);
    const char* go_forward(const char* p);
    const char* skip_blankspaces(const char* p);
    const char* goto_first_blankspace(const char* p);
    const char* next_token(const char* p);
    const char* goto_char(const char ch, const char* p);
    const char* goto_first_of(const char * chars, const char* p);
    const char* goto_eol(const char* p);
    const char* goto_nextline(const char* p);
    const char* goto_commentblock_end(const char* p);
    const char* find_closing_bracket(const char* p);
    const char* try_token(const char* p);
    unsigned int count_characters(const char* from, const char* to, const char ch);
    unsigned int get_line(const char* to);

    const char* process_comment(const char* p);
    const char* process_macro(const char* p);
    const char* process_typedef(const char* p);
    const char* process_enum(const char* p);
    const char* process_struct(const char* p);
    const char* process_struct_body(const char* p);
    const char* process_function(const char* p);
    const char* process_function_args(const char* p);
    const char* process_function_body(const char* p);
};