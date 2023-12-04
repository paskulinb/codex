#include <string>
#include <deque>
#include <vector>
#include <map>
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
    unsigned int file_length; 
    bool preproc_done;
    std::vector<std::string> tokens;
    std::deque<Ct> level;
    std::vector<std::string> functions;
    unsigned int parenthesis_level;
    bool line_first_nonspace_found;

    std::map<std::string, std::string> defs;
    std::vector<std::string> includes;

    Ct& ct_top() { return level.back(); }
    bool ct_top_is(Ct::Type type) { return type == level.back().type(); }
    Ct& ct_push(Ct construct);
    Ct& ct_push(Ct::Type type);
    void ct_pop(unsigned int count = 1);

public:
    Parser();
    void parse(std::string filename);
    char* ReadFile(std::string filename);
    void SaveFile(std::string filename);
    bool isspecifier(const char ch);
    bool ispath(const char ch);
    bool isalnum_us(const char ch);
    bool isalpha_us(const char ch);
    bool is_first(const char*);
    bool is_last(const char*);
    const char* proceed(const char* p);
    const char* pass_blankspaces(const char* p);
    const char* skip_horizontalspaces(const char* p);
    const char* goto_first_blankspace(const char* p);
    const char* increment(const char **p);
    const char* next_token(const char* p);
    const char* goto_char(const char ch, const char* p);
    const char* goto_first_of(const char * chars, const char* p);
    const char* goto_eol(const char* p);
    const char* goto_closing_quote(const char* p);
    const char* goto_nextline(const char* p);
    const char* goto_commentblock_end(const char* p);
    const char* find_closing_bracket(const char* p);
    const char* try_token(const char* p, std::string& token);
    const char* try_path(const char* p, std::string& path);
    unsigned int count_characters(const char* from, const char* to, const char ch);
    void set_blank(char* first, char* last);
    unsigned int get_line(const char* to);
    void evt_first_char_in_line(const char* p);
    void evt_last_char_in_line(const char* p);

    const char* process_comment(const char* p);
    const char* process_preproc(const char* p);
    const char* process_string_literal(const char* p);
    const char* process_extern(const char* p);
    const char* process_typedef(const char* p);
    const char* process_enum(const char* p);
    const char* process_struct(const char* p);
    const char* process_struct_body(const char* p);
    const char* process_function(const char* p);
    const char* process_function_args(const char* p);
    const char* process_function_body(const char* p);

    void dump_level(void);
};