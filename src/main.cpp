#include "Version.h"
#include "js.h"
#include "Frontend.h"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc,char**argv){
 if(argc==2&&std::string(argv[1])=="--version"){std::cout<<"JS++ "<<js_version()<<"\n";return 0;}
 if(argc>=3&&std::string(argv[1])=="--check"){
  std::string source;if(std::string(argv[2])=="-e"){if(argc!=4){std::cerr<<"js: --check -e requires source\n";return 2;}source=argv[3];}else{std::ifstream in(argv[2],std::ios::binary);if(!in){std::cerr<<"js: cannot read "<<argv[2]<<"\n";return 2;}source.assign(std::istreambuf_iterator<char>(in),{});}
  jspp::Program program;jspp::Diagnostic diagnostic;if(!jspp::parse_source(source,program,diagnostic)){std::cerr<<"js: "<<jspp::format_diagnostic(diagnostic)<<"\n";return 2;}return 0;
 }
 std::cerr<<"js: evaluation is not implemented yet\n";
 return 2;
}
