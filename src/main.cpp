#include "Version.h"
#include "js.h"
#include <iostream>
#include <string>

int main(int argc,char**argv){
 if(argc==2&&std::string(argv[1])=="--version"){std::cout<<"JS++ "<<js_version()<<"\n";return 0;}
 std::cerr<<"js: evaluation is not implemented yet\n";
 return 2;
}
