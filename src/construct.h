#include <string>


class Construct
{
public:
    enum Type {
        CT_NONE,
        CT_EXTERN,
        CT_COMMENT,
        CT_COMMENT_LINE,
        CT_COMMENT_BLOCK,
        CT_MACRO,
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
    };

private:
	Type m_type;
	std::string m_name;

public:
	Construct() = delete;
	Construct(Type type, std::string name = "");
	~Construct();
	Type type() { return m_type; };
    std::string name() { return m_name; };
};