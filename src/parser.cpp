#include "parser.h"
#include <iostream>
#include <fstream>
#include <cctype>

Parser::Parser()
{
}

bool Parser::isspecifier(const char ch)
{
    return isalnum(ch) || ch == '_' || ch == '*';
}
bool Parser::isalnum_us(const char ch)
{
    return isalnum(ch) || ch == '_';
}
bool Parser::isalpha_us(const char ch)
{
    return isalpha(ch) || ch == '_';
}

const char* Parser::skip_blankspaces(const char* p)
{
    while (('\0' != *p) && isspace(*p))
    {
        ++p;
    }
    return p;
}

const char* Parser::goto_first_blankspace(const char* p)
{
    while (!isspace(*p)) ++p;
    return p;
}

const char* Parser::next_token(const char* p)
{
    p = goto_first_blankspace(p);
    p = skip_blankspaces(p);
    return p;
}

const char* Parser::goto_char(const char ch, const char* p)
{
    if (nullptr == p) return nullptr;

    while (*p != ch)
    {
        if (*(++p) == '\0') return nullptr;
    }
    return p;
}

const char* Parser::goto_first_of(const char* chars, const char* p)
{
    unsigned int nc = strlen(chars);
    
    while (true)
    {
        if (*p == '\0') return nullptr;

        for (unsigned int i = 0; i < nc; ++i)
        {
            if (*(chars + i) == *p) return p;
        }
        ++p;
    }

    return nullptr;
}

const char* Parser::goto_eol(const char* p)
{
    if (nullptr == p) return nullptr;

    while ((*p != '\n') && (*p != '\r'))
    {
        if (*(++p) == '\0') return nullptr;
    }
    while ((*(p+1) == '\n') || (*(p+1) == '\r'))
    {
        if (*(++p) == '\0') return nullptr;
    }
    return p;
}

const char* Parser::goto_nextline(const char* p)
{
    if (nullptr == p) return nullptr;

    while ((*p != '\n') && (*p != '\r'))
    {
        if (*(++p) == '\0') return nullptr;
    }
    while ((*p == '\n') || (*p == '\r'))
    {
        if (*(++p) == '\0') return nullptr;
    }
    return p;
}

const char* Parser::goto_commentblock_end(const char* p)
{
    if (nullptr == p) return nullptr;

    if (nullptr == (p = strstr(p, "*/")))
        return nullptr;

    ++p;

    return p;
}

const char* Parser::find_closing_bracket(const char* p)
{
    unsigned int depth = 1;
    const char Open = *p;
    char Close = '\0';

    if      ('(' == Open) Close = ')';
    else if ('{' == Open) Close = '}';
    else if ('[' == Open) Close = ']';
    else return p;
    
    while (0 < depth)
    {
        ++p;

        if ('\0' == *p)
        {
            return nullptr;
        }

        if ('/' == *p)
        {
            ct_push(Ct::CT_COMMENT);
            p = process_comment(p);
        }

        if      (Close == *p) depth--;
        else if (Open == *p) depth++;
    }
    return p;
}

unsigned int Parser::count_characters(const char* from, const char* to, const char ch)
{
    unsigned int count = 0;
    while (from <= to)
    {
        if (ch == *from)
        {
            ++count;
        }
        ++from;
    }
    return count;
}

unsigned int Parser::get_line(const char* to)
{
    return 1 + count_characters(file_buff, to, '\n');
}



char* Parser::ReadFile(std::string filename)
{
    std::ifstream ifs(filename);
    if (!ifs) return "";

    ifs.seekg(0, ifs.end);
    long len = ifs.tellg();

    ifs.seekg(0, ifs.beg);

    char* buff = new char[len + 1]();
    ifs.read(buff, len);
    ifs.close();

    return buff;
}

void Parser::parse(std::string filename)
{
    std::cout << "############## " << filename << " ###############" << std::endl;
    file_buff = ReadFile(filename);
    const char* P = file_buff;


    level = std::stack<Ct>();
    ct_push(Ct::CT_NONE);

    try
    {
        while (nullptr != P)
        {
            P = go_forward(P);
            

            switch (ct_top().type())
            {
            case Ct::CT_EXTERN:
                P = process_extern(P);
                break;

            case Ct::CT_MACRO:
                P = process_macro(P);
                break;

            case Ct::CT_COMMENT:
                P = process_comment(P);
                break;

            case Ct::CT_TYPEDEF:
                P = process_typedef(P);
                break;

            case Ct::CT_ENUM:
                P = process_enum(P);
                break;

            case Ct::CT_STRUCT:
                P = process_struct(P);
                break;

            case Ct::CT_STRUCT_BODY:
                P = process_struct_body(P);
                break;

            case Ct::CT_FUNCTION:
                P = process_function(P);
                break;

            case Ct::CT_FUNCTION_ARGS:
                P = process_function_args(P);
                break;

            case Ct::CT_FUNCTION_BODY:
                P = process_function_body(P);
                break;
            }
        }
    }
    catch (PExcept e)
    {
        switch (e)
        {
        case PExcept::E_UNRESOLVED:
            std::cout << "Unresolved: " << " line:" << count_characters(file_buff, P, '\n') + 1 << " construct type:" << level.top().type() << std::endl;
            break;

        case PExcept::E_EOF:
            std::cout << "Finished" << std::endl;
            break;
        }
    }

    delete[] file_buff;
}

const char* Parser::go_forward(const char* p)
{
    p = skip_blankspaces(p);
    
    auto line = get_line(p);
    std::cout << "L:" << line << std::endl;

    if ('\0' == *p)
    {
        throw PExcept::E_EOF;
    }

    switch (*p)
    {
    case '#':

        if (Ct::CT_STRING_LITERAL != ct_top().type())
        {
            ct_push(Ct::CT_MACRO);
            return p;
        }
        break;

    case '/':

        if (Ct::CT_STRING_LITERAL != ct_top().type())
        {
            ct_push(Ct::CT_COMMENT);
            return p;
        }
        break;

    case '(':

        if (Ct::CT_NONE == ct_top().type())
        {
            if (!tokens.empty())
            {
                ct_push(Ct(Ct::CT_FUNCTION, tokens.back()));
                ct_push(Ct::CT_FUNCTION_ARGS);
            }
            return p;
        }
        break;

    case ')':

        if (Ct::CT_FUNCTION_ARGS == ct_top().type())
        {
            //ct_pop();
            //return ++p;
        }
        break;

    case '{':

        switch (ct_top().type())
        {
        case Ct::CT_FUNCTION:
            ct_push(Ct::CT_FUNCTION_BODY);
            return p;
            break;

        case Ct::CT_STRUCT:
            ct_push(Ct::CT_STRUCT_BODY);
            return p;
            break;

        default:
            ct_push(Ct::CT_CODE_BLOCK);
            return p;
            break;
        }
        break;

    case '}':

        switch (ct_top().type())
        {
        case Ct::CT_FUNCTION_BODY:
            tokens.clear();
            ct_pop();
            ct_pop(); //exit also CT_FUNCTION level
            return ++p;
            break;

        case Ct::CT_STRUCT:
            ct_pop();
            return ++p;
            break;

        case Ct::CT_CODE_BLOCK:
            ct_pop();
            return ++p;
            break;

        default:
            break;
        }
        break;

    case ';':

        if (Ct::CT_FUNCTION == ct_top().type())
        {
            std::cout << "function declaration" << std::endl;
            ct_pop();
            tokens.clear();
            return ++p;
        }
        else if (Ct::CT_STRUCT == ct_top().type())
        {
            ct_pop();
            tokens.clear();
            return ++p;
        }
        break;

    case '=':

        /* variable/struct/class initialization */
        std::cout << "variable/struct/class initialization" << std::endl;
        p = goto_char(';', p);
        tokens.clear();
        return ++p;
        break;
    }

    /* Token: Identifier, Specifier, Punctuator, ... */
    p = try_token(p);
    
    if (!token.empty())
    {
        tokens.push_back(token);

        if ("typedef" == token)
        {
            ct_push(Ct::CT_TYPEDEF);
        }
        else if ("struct" == token)
        {
            ct_push(Ct::CT_STRUCT);
        }
        else if ("enum" == token)
        {
            ct_push(Ct::CT_ENUM);
        }
        else if ("extern" == token)
        {
            ct_push(Ct::CT_EXTERN);
        }
        return p;
    }

    throw PExcept::E_UNRESOLVED;
    
    return nullptr;
}

const char* Parser::process_comment(const char* p)
{
    switch (*(p + 1))
    {
    case '/':
        std::cout << "L_COMMENT_LINE" << std::endl;
        ct_push(Ct::CT_COMMENT_LINE);
        p = goto_eol(p);
        ct_pop();
        break;

    case '*':
        std::cout << "L_COMMENT_BLOCK" << std::endl;
        ct_push(Ct::CT_COMMENT_BLOCK);
        p = goto_commentblock_end(p);
        ++p;
        ct_pop();
        break;
    }
    ct_pop(); //exit CT_COMMENT
    return p;
}

const char* Parser::process_macro(const char* p)
{
    std::cout << "L_MACRO ";
    if (try_token(++p))
    {
        std::cout << token;
    }
    std::cout << std::endl;
    p =  goto_eol(p);
    ct_pop();
    return p;
}

const char* Parser::process_extern(const char* p)
{
    std::cout << "L_EXTERN" << std::endl;
    p = goto_nextline(p);
    ct_pop();
    tokens.clear();
    return p;
}

const char* Parser::process_typedef(const char* p)
{
    std::cout << "L_TYPEDEF" << std::endl;
    p = goto_char(';', p);
    tokens.clear();
    ct_pop();
    return p;
}

const char* Parser::process_enum(const char* p)
{
    std::cout << "L_ENUM"  << std::endl;
    p = goto_char(';', p);
    tokens.clear();
    ct_pop();
    return p;
}

const char* Parser::process_struct(const char* p)
{
    std::cout << "L_STRUCT " << std::endl;
    return p;
}

const char* Parser::process_struct_body(const char* p)
{
    std::cout << "L_STRUCT_BODY" << std::endl;
    p = goto_char(';', p);
    ct_pop();
    ct_pop();
    return p;
}



const char* Parser::process_function(const char* p)
{
    return p;
}

const char* Parser::process_function_args(const char* p)
{
    std::cout << "L_FUNCTION_ARGS" << std::endl;
    p = find_closing_bracket(p);
    ct_pop();
    ++p;
    return p;
}

const char* Parser::process_function_body(const char* p)
{
    std::cout << "L_FUNCTION_BODY" << std::endl;
    p = find_closing_bracket(p);
    tokens.clear();
    ct_pop();
    ct_pop(); //exit also CT_FUNCTION level
    ++p;
    return p;
}

const char* Parser::try_token(const char* p)
{
    const char * p1 = p;
    while (('\0' != *p) && isspecifier(*p)) ++p;
    token = std::string(p1, p-p1);
    return p;
}

void Parser::ct_push(Ct construct)
{
    std::cout << ">> C.type:" << construct.type() << " C.name:" << construct.name() << std::endl;
    level.push(construct);
}

void Parser::ct_push(Ct::Type type)
{
    std::cout << ">> C.type:" << type << std::endl;
    level.push(Ct(type));
}

void Parser::ct_pop()
{
    level.pop();
}

