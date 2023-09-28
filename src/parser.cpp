#include "parser.h"
#include <iostream>
#include <fstream>
#include <cctype>

Parser::Parser()
{
    state = S_BASE;
    parenthesis_level = 0;
}

bool Parser::isname(const char ch)
{
    return isalnum(ch) || ch == '_';
}

bool nextline(const char* p)
{
    while (*p != '\n' && *p != '\r') p++;
    while (*p == '\n' || *p == '\r') p++;
    return true;
}

std::string Parser::ReadFile(std::string filename)
{
    std::ifstream ifs(filename);
    if (!ifs) return "";

    ifs.seekg(0, ifs.end);
    long len = ifs.tellg();

    ifs.seekg(0, ifs.beg);

    char* buff = new char[len + 1];
    ifs.read(buff, len);
    ifs.close();

    buff[len] = '\0';

    std::string out(buff);

    delete[] buff;

    return out;
}

void Parser::Parse(std::string filename)
{
    std::cout << "############## " << filename << " ###############" << std::endl;
    auto file = ReadFile(filename);
    const char* body = file.c_str();
    const char* P = body;


    state = S_BASE;
    
    while (isspace(*P++));

    switch (*P)
    {
    case '#':
        state = S_MACRO;
        std::cout << "Macro def" << std::endl;
        nextline(P);
        break;

    case '/':
        state = S_COMMENT;
        std::cout << "Comment" << std::endl;

    default:

    }
}
