#include <string>


class Construct
{
public:
    enum Type {
        CT_NONE,
        CT_EXTERN,
        CT_COMMENT_LINE,
        CT_COMMENT_BLOCK,
        CT_PREPROC,
        CT_TYPEDEF,
        CT_ENUM,
        CT_STRUCT,
        CT_STRUCT_BODY,       // {...}
        CT_FUNCTION,
        CT_FUNCTION_ARGS,
        CT_FUNCTION_BODY,
        CT_STRING_LITERAL,
        CT_ROUND_BRACKET,
        CT_CODE_BLOCK,
        CT_ASSIGNMENT,
        CT__KEEP__,
    };
    enum SubType {
        CST_NONE,
        CST_PREPROC_include,
        CST_PREPROC_ifdef,
        CST_PREPROC_ifndef,
    };

private:
	Type m_type;
    SubType m_subtype;
    std::string m_name;
	bool m_condition;
    char* m_start;
    char* m_end;

public:
	Construct() = delete;
	Construct(Type type, std::string name = "");
	~Construct();
    Construct& type_change(Type type, SubType subtype = Construct::SubType::CST_NONE);
	Type type();
	Type operator()();
    SubType subtype();
    Construct& name(std::string name);
    std::string name();
    Construct& condition(bool condition);
    bool condition(void);
    Construct& mark_start(const char* p);
    char* mark_start(void);
    Construct& mark_end(const char* p);
    char* mark_end(void);
};