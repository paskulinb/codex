#include <string>
#include <stack>
#include <vector>

class Parser
{
private:
    
    enum Level {
        L_BASE,
        L_COMMENT,
        L_COMMENT_LINE,
        L_COMMENT_BLOCK,
        L_MACRO,
        L_TYPEDEF,
        L_ENUM,
        L_STRUCT,
        L_STRUCT_BODY,       // {...}
        L_IDENTIFIER,
        L_FUNCTION,
        L_FUNCTION_ARGS,
        L_FUNCTION_BODY,
        L_STRING_LITERAL,
        L_ROUND_BRACKET,
    };

    const char* file_start;
    std::vector<std::string> hold;
    std::stack<Level> level;
    std::vector<std::string> functions;
    std::string word;
    unsigned int parenthesis_level;

public:
    Parser();
    void Parse(std::string filename);
    char* ReadFile(std::string filename);
    bool isspecifier(const char ch);
    bool isalnum_us(const char ch);
    bool isalpha_us(const char ch);
    const char* skip_blankspaces(const char* p);
    const char* goto_first_blankspace(const char* p);
    const char* next_token(const char* p);
    const char* goto_char(const char ch, const char* p);
    const char* goto_first_of(const char * chars, const char* p);
    const char* goto_nextline(const char* p);
    const char* goto_commentblock_passend(const char* p);
    const char* find_closing_bracket(const char* p);
    const char* determine_word(const char* p);
    const char* determine_specifier(const char* p);
    unsigned int count_characters(const char* from, const char* to, const char ch);

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