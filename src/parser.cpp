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

const char* Parser::goto_commentblock_passend(const char* p)
{
    if (nullptr == p) return nullptr;

    if (nullptr == (p = strstr(p, "*/")))
        return nullptr;

    p += 2;
    if ('\0' == *p)
        return nullptr;

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

void Parser::Parse(std::string filename)
{
    std::cout << "############## " << filename << " ###############" << std::endl;
    file_start = ReadFile(filename);
    const char* P = file_start;


    level = std::stack<Level>();
    level.push(L_BASE);

    while (true)
    {
        P = skip_blankspaces(P);
        
        if ('\0' == *P) break;

        switch (*P)
        {
        case '#':
            P = process_macro(P);
            continue;
            break;

        case '/':
            if (L_STRING_LITERAL != level.top())
            {
                P = process_comment(P);
                continue;
            }
            break;

        case '(':
            if (!hold.empty())
            {
                P = process_function(P);
                continue;
            }
            break;

        case '=':
            /* initialization */
            std::cout << "variable/struct/class initialization" << std::endl;
            P = goto_char(';', P);
            ++P;
            hold.clear();
            continue;
            break;
        }

        /* Identifier, Specifier */
        P = determine_specifier(P);
        if (!word.empty())
        {
            hold.push_back(word);

            if ("typedef" == word)
            {
                P = process_typedef(P);
                continue;
            }
            else if ("struct" == word)
            {
                P = process_struct(P);
                continue;
            }
            else if ("enum" == word)
            {
                P = process_enum(P);
                continue;
            }
            else if ("extern" == word)
            {
                P = goto_nextline(P);
                continue;
            }

            continue;
        }

        std::cout << "Unresolved: " << " line:" << count_characters(file_start, P, '\n') + 1 << " level:" << level.top() << std::endl;
        return;
    }
    
    std::cout << "Finished" << std::endl;

    delete[] file_start;
}

const char* Parser::process_comment(const char* p)
{
    switch (*(p + 1))
    {
    case '/':
        std::cout << "L_COMMENT_LINE" << std::endl;
        level.push(L_COMMENT_LINE);
        p = goto_nextline(p);
        level.pop();
        break;

    case '*':
        std::cout << "L_COMMENT_BLOCK" << std::endl;
        level.push(L_COMMENT_BLOCK);
        p = goto_commentblock_passend(p);
        level.pop();
        break;
    }
    return p;
}

const char* Parser::process_macro(const char* p)
{
    std::cout << "L_MACRO ";
    if (determine_word(++p))
    {
        std::cout << word;
    }
    std::cout << std::endl;
    level.push(L_MACRO);
    p =  goto_nextline(p);
    level.pop();
    return p;
}

const char* Parser::process_typedef(const char* p)
{
    std::cout << "L_TYPEDEF" << std::endl;
    level.push(L_TYPEDEF);
    p = goto_char(';', p);
    hold.clear();
    level.pop();
    return p;
}

const char* Parser::process_enum(const char* p)
{
    std::cout << "L_ENUM"  << std::endl;
    level.push(L_ENUM);
    p = goto_char(';', p);
    hold.clear();
    level.pop();
    return p;
}

const char* Parser::process_struct(const char* p)
{
    std::string struct_name = hold.back();
    std::cout << "L_STRUCT " << struct_name << std::endl;
    level.push(L_STRUCT);
    p = skip_blankspaces(p);
    if ('{' == *p)
    {
        p = process_struct_body(p);
        /* this was struct definition */
        p = goto_char(';', p);
    }
    else
    {
        if (L_STRUCT_BODY == level.top())
        {
            /* this was struct ref */
        }
        else
        {
            /* this was struct declaration */
        }
    }
    hold.clear();
    level.pop();
    return p;
}

const char* Parser::process_struct_body(const char* p)
{
    std::cout << "L_STRUCT_BODY" << std::endl;
    level.push(L_STRUCT_BODY);
    p = find_closing_bracket(p);
    level.pop();
    return p;
}



const char* Parser::process_function(const char* p)
{
    std::string function_name = hold.back();
    std::cout << "L_FUNCTION " << function_name << std::endl;
    level.push(L_FUNCTION);
    p = process_function_args(p);
    p = skip_blankspaces(p);
    if ('{' == *p)
    {
        p = process_function_body(p);
        /* this was function definition */
        std::cout << "function definition" << std::endl;
    }
    else if (';' == *p)
    {
        /* this was function declaration */
        std::cout << "function declaration" << std::endl;
    }
    else
    {
        /* this was function call */
        std::cout << "function call" << std::endl;
    }
    hold.clear();
    ++p;
    level.pop();
    return p;
}

const char* Parser::process_function_args(const char* p)
{
    std::cout << "L_FUNCTION_ARGS" << std::endl;
    level.push(L_FUNCTION_ARGS);
    p = find_closing_bracket(p);
    ++p;
    level.pop();
    return p;
}

const char* Parser::process_function_body(const char* p)
{
    std::cout << "L_FUNCTION_BODY" << std::endl;
    level.push(L_FUNCTION_BODY);
    p = find_closing_bracket(p);
    level.pop();
    return p;
}

const char* Parser::determine_word(const char* p)
{
    const char * p1 = p;
    while (isalnum_us(*p)) ++p;
    word = std::string(p1, p-p1);
    //std::cout << word << std::endl;
    return p;
}

const char* Parser::determine_specifier(const char* p)
{
    const char * p1 = p;
    while (isspecifier(*p)) ++p;
    word = std::string(p1, p-p1);
    //std::cout << word << std::endl;
    return p;
}