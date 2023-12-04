#include "parser.h"
#include <iostream>
#include <fstream>
#include <cctype>

Parser::Parser()
{
}

void Parser::parse(std::string filename)
{
    std::cout << "############## " << filename << " ###############" << std::endl;
    file_buff = ReadFile(filename);
    const char* P = file_buff;

    level = std::deque<Ct>();
    ct_push(Ct::CT_NONE);

    try
    {
        /* Pre Proc */
        preproc_done = false;
        line_first_nonspace_found = false;

        while (true)
        {
            P = proceed(P);

            switch (ct_top()())
            {
            case Ct::CT_PREPROC:
                dump_level();
                P = process_preproc(P);
                break;
            }
        }
    }
    catch (PExcept e)
    {
        switch (e)
        {
        case PExcept::E_UNRESOLVED:
            std::cout << "Unresolved: " << " line:" << count_characters(file_buff, P, '\n') + 1 << " construct type:" << level.back().type() << std::endl;
            break;

        case PExcept::E_EOF:
            std::cout << "End of file" << std::endl;
            break;
        }
    }

    dump_level();
    SaveFile("C:/Users/paskulin/OneDrive - abrantix AG/Desktop/codex/test/out.c");

    try
    {
        /* Main Proc */
        P = file_buff;
        preproc_done = true;

        while (true)
        {
            P = proceed(P);

            switch (ct_top()())
            {
            case Ct::CT_STRING_LITERAL:
                P = process_string_literal(P);
                break;

            case Ct::CT_EXTERN:
                P = process_extern(P);
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
            std::cout << "Unresolved: " << " line:" << count_characters(file_buff, P, '\n') + 1 << " construct type:" << level.back().type() << std::endl;
            std::cin.get();
            break;

        case PExcept::E_EOF:
            std::cout << "End of file" << std::endl;
            std::cin.get();
            break;
        }
    }

    delete[] file_buff;
}

const char* Parser::proceed(const char* p)
{
    p = pass_blankspaces(p);
    
    while (!preproc_done)
    {
        if (('#' == *p) && !line_first_nonspace_found)
        {
            ct_push(Ct::CT_PREPROC);
            return p;
        }
        increment(&p);
    }

    switch (*p)
    {
    case '(':

        if (Ct::CT_NONE == ct_top()())
        {
            if (!tokens.empty())
            {
                ct_push(Ct::CT_FUNCTION).name(tokens.back());
                ct_push(Ct::CT_FUNCTION_ARGS);
            }
            return p;
        }
        break;

    case '{':

        switch (ct_top()())
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

    case '"':

        if (Ct::CT_COMMENT_LINE != ct_top()() &&
            Ct::CT_COMMENT_BLOCK != ct_top()())
        {
            ct_push(Ct::CT_STRING_LITERAL);
            return p;
        }
        break;

    case ';':

        if (Ct::CT_FUNCTION == ct_top()())
        {
            ct_pop();
            tokens.clear();
            increment(&p);
            return p;
        }
        else if (Ct::CT_STRUCT == ct_top()())
        {
            ct_pop();
            tokens.clear();
            increment(&p);
            return p;
        }
        break;

    case '=':

        if ('=' != *(p + 1))
        {
            ct_push(Ct::CT_ASSIGNMENT);
        }
        p = goto_char(';', p);
        tokens.clear();
        increment(&p);
        return p;
        break;
    }

    /* Token: Identifier, Specifier, Punctuator, ... */
    std::string token;
    p = try_token(p, token);
    
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
        increment(&p);
        ct_pop();
        break;

    default:
        std::cout << "UNKNOWN" << std::endl;

    }
    ct_pop(); //exit CT_COMMENT
    return p;
}

const char* Parser::process_string_literal(const char* p)
{
    p = goto_closing_quote(p);
    ct_pop();
    return p;
}

const char* Parser::process_preproc(const char* p)
{
    char* mod;
    std::string directive;
    std::string identifier;
    std::string token_string;
    std::string path_spec;

    if ('#' == *p) // preproc directive start
    {
        const char* start = p;
        const char* end = p;

        std::cout << "#";
        increment(&p);
        p = pass_blankspaces(p);
        p = try_token(p, directive);
        std::cout << directive;

        if ("include" == directive)
        {
            //p = skip_horizontalspaces(p);
            p = goto_first_of("<\"", p);
            increment(&p);
            p = skip_horizontalspaces(p);
            p = try_path(p, path_spec);
            p = goto_first_of(">\"", p);
            increment(&p);
            std::cout << " " << path_spec;
            includes.push_back(path_spec);
            set_blank(const_cast<char*>(start), const_cast<char*>(p));
            ct_pop();
        }
        else if ("define" == directive)
        {
            p = skip_horizontalspaces(p);
            p = try_token(p, identifier);
            std::cout << identifier;
            p = skip_horizontalspaces(p);
            p = try_token(p, token_string);
            std::cout << " " << token_string;
            defs.insert(std::pair<std::string, std::string>(identifier, token_string));
            p = goto_eol(p);
            set_blank(const_cast<char*>(start), const_cast<char*>(p));
            ct_pop();
        }
        else if ("ifdef" == directive)
        {
            p = skip_horizontalspaces(p);
            p = try_token(p, identifier);
            std::cout << " " << identifier;
            ct_top()
                .type_change(Ct::Type::CT__KEEP__, Ct::SubType::CST_PREPROC_ifdef)
                .condition(defs.find(identifier) != defs.end())
                .mark_start(start)
                .mark_end(p);
            set_blank(const_cast<char*>(start), const_cast<char*>(p));
        }
        else if ("ifndef" == directive)
        {
            p = skip_horizontalspaces(p);
            p = try_token(p, identifier);
            std::cout << " " << identifier;
            ct_top()
                .type_change(Ct::Type::CT__KEEP__, Ct::SubType::CST_PREPROC_ifndef)
                .condition(defs.find(identifier) == defs.end())
                .mark_start(start)
                .mark_end(p);
            set_blank(const_cast<char*>(start), const_cast<char*>(p));
        }
        else if ("endif" == directive)
        {
            ct_pop(); // #endif level

            char* D = ct_top().mark_start();

            if (false == ct_top().condition())
            {
                set_blank(ct_top().mark_end(), const_cast<char*>(p));
            }
            else
            {
                set_blank(const_cast<char*>(start), const_cast<char*>(p));
            }
            
            ct_pop(); // #ifdef, #ifndef level
        }
        
        std::cout << std::endl;
    }

    return p;
}

void Parser::set_blank(char* first, char* last)
{
    for (char* c = first; c <= last; ++c)
    {
        if (!isspace(*c)) *c = ' ';  //preserve existing blankspaces
    }
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
    increment(&p);
    return p;
}

const char* Parser::process_function_body(const char* p)
{
    std::cout << "L_FUNCTION_BODY" << std::endl;
    p = find_closing_bracket(p);
    tokens.clear();
    ct_pop();
    ct_pop(); //exit also CT_FUNCTION level
    increment(&p);
    return p;
}

Ct& Parser::ct_push(Ct construct)
{
    level.push_back(construct);
    return level.back();
}

Ct& Parser::ct_push(Ct::Type type)
{
    level.push_back(Ct(type));
    return level.back();
}

void Parser::ct_pop(unsigned int count)
{
    while (0 < count--)
    {
        level.pop_back();
    }
}

bool Parser::isspecifier(const char ch)
{
    return isalnum(ch) || ch == '_' || ch == '*';
}
bool Parser::ispath(const char ch)
{
    return isalnum(ch) || ch == '_' || ch == '.' || ch == '/' || ch == '\\';
}
bool Parser::isalnum_us(const char ch)
{
    return isalnum(ch) || ch == '_';
}
bool Parser::isalpha_us(const char ch)
{
    return isalpha(ch) || ch == '_';
}

bool Parser::is_first(const char* p)
{
    if (('\n' != *p) && ('\n' == *(p - 1))) return true;
    return false;
}

bool Parser::is_last(const char* p)
{
    if (('\n' != *p) && ('\n' == *(p + 1))) return true;
    return false;
}

const char* Parser::pass_blankspaces(const char* p)
{
    while (isspace(*p))
    {
        increment(&p);
    }
    return p;
}

void Parser::evt_first_char_in_line(const char* p)
{
    auto line = get_line(p);
    std::cout << "L:" << line << std::endl;

}

void Parser::evt_last_char_in_line(const char* p)
{

}

const char* Parser::skip_horizontalspaces(const char* p)
{
    while ((' ' == *p) || ('\t' == *p))
    {
        increment(&p);
    }
    return p;
}

const char* Parser::try_token(const char* p, std::string& token)
{
    const char * p1 = p;
    while (('\0' != *p) && isspecifier(*p)) increment(&p);
    token.assign(p1, p - p1);
    return p;
}

const char* Parser::try_path(const char* p, std::string& token)
{
    const char * p1 = p;
    while (ispath(*p)) increment(&p);
    token.assign(p1, p - p1);
    return p;
}

const char* Parser::goto_first_blankspace(const char* p)
{
    while (!isspace(*p)) increment(&p);
    return p;
}

const char* Parser::increment(const char **p)
{
    ++(*p);
    
    if ('\0' == **p) throw PExcept::E_EOF;

    if (is_first(*p))
    {
        evt_first_char_in_line(*p);
    }

    if (is_last(*p))
    {
        evt_last_char_in_line(*p);
    }

    if ('\n' == **p)
    {
        line_first_nonspace_found = false;
    }

    if (('\t' == **p) || (' ' == **p))
    {
        line_first_nonspace_found = true;
    }

    if ('/' == **p)
    {
        switch (*(*p + 1))
        {
        case '/':
            *p = goto_eol(*p);
            break;
        case '*':
            *p = goto_commentblock_end(*p);
            increment(p);
            break;
        default:
//           (*p)++;
            break;
        }
    }

    return *p;
}

const char* Parser::next_token(const char* p)
{
    p = goto_first_blankspace(p);
    p = pass_blankspaces(p);
    return p;
}

const char* Parser::goto_char(const char ch, const char* p)
{
    while (*p != ch)
    {
        increment(&p);
    }
    return p;
}

const char* Parser::goto_first_of(const char* chars, const char* p)
{
    unsigned int nc = strlen(chars);

    while (true)
    {
        for (unsigned int i = 0; i < nc; ++i)
        {
            if (*(chars + i) == *p) return p;
        }
        increment(&p);
    }

    return p;
}

const char* Parser::goto_eol(const char* p)
{
    if (nullptr == p) return nullptr;

    while (true)
    {
        switch (*(p+1))
        {
        case '\n':
        case '\r':
            return p;
        }
        increment(&p);
    }
    return p;
}

const char* Parser::goto_closing_quote(const char* p)
{
    if ((*p != '"') && (*p != '\'')) return p;

    char quote = *p;

    while (true)
    {
        increment(&p);
        p = goto_char(quote, p);
        if (*(p-1) == '\\' && *(p-2) != '\\')
        {
            continue;
        }
        else
        {
            break;
        }
    }
    return p;
}

const char* Parser::goto_nextline(const char* p)
{
    if (nullptr == p) return nullptr;

    while ((*p != '\n') && (*p != '\r'))
    {
        increment(&p);
    }
    while ((*p == '\n') || (*p == '\r'))
    {
        increment(&p);
    }
    return p;
}

const char* Parser::goto_commentblock_end(const char* p)
{
    while (true)
    {
        increment(&p);
        if (('*' == *p) && ('/' == *(p + 1)))
        {
            break;
        }
    }

    increment(&p);
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
        increment(&p);

        if ('\0' == *p)
        {
            return nullptr;
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

    char* buff = new char[len + 2]();
    ifs.read(buff + 1, len);
    *buff = ' '; //Make firs char blank space. At this point increment() will start
    ifs.close();

    file_length = strchr(buff, '\0') - buff - 1;

    return buff;
}

void Parser::SaveFile(std::string filename)
{
    std::ofstream outfile(filename, std::ios::out);
    outfile << std::string(file_buff + 1, file_length);
    outfile.close();
}

void Parser::dump_level(void)
{
    std::cout << "level [";
    for (auto it = level.begin(); it != level.end(); it++)
        std::cout << it->type() << ",";
    std::cout << "]" << std::endl;
}