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
        p.Parse(filename);
    }
    else
    {
        //std::string file("C:/Users/paskulin/source/repos/Projects.SPS.MSW2/src/application/ep2/ep2.h");
        std::string file("C:/Users/paskulin/source/repos/Projects.SPS.MSW2/src/application/ep2/ep2_app.c");
        p.Parse(file);
    }

    return 0;
}