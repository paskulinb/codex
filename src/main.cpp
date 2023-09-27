#include "parser.h"
#include <iostream>
#include <fstream>

int main(int argc, char **argv)
{
    Parser p;
    
    if (argc > 1)
    for (int i = 1; i < argc; ++i)
    {
        char* filename = *(argv+i);
        std::cout << filename << std::endl;
        std::ifstream ifs (*(argv+i), std::ifstream::in);
        p.Parse(filename);
    }

    return 0;
}