
#include<stdio.h>
#include <iostream>
#include<fstream>
#include<sstream>
#include "walker.h"

//  clang++ main.cpp walker.cpp token.cpp scanner.cpp visitor.cpp codegen.cpp frame.cpp parser.cpp Ast.cpp -o main




int main(int, char**) {
    std::ifstream ifs;
    ifs.open("text.c");
    std::ostringstream tmp;
    tmp << ifs.rdbuf();
    std::cout << "Reading from the file" << std::endl;
    std::string file_str = tmp.str();
    char* file_source = (char*)file_str.data();
    std::cout << file_source << std::endl;
    WalkerTree* walk = new WalkerTree();
    walk->walkerRun(file_source);
    return 0;
}





