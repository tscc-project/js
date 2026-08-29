#include "Version.h"
#include "js.h"
#include "Frontend.h"
#include "Bytecode.h"
#include <fstream>
#include <iostream>
#include <string>

int main(int argc,char**argv){
 if(argc==2&&std::string(argv[1])=="--version"){std::cout<<"JS++ "<<js_version()<<"\n";return 0;}
 if(argc>=3&&std::string(argv[1])=="--check"){
  std::string source;if(std::string(argv[2])=="-e"){if(argc!=4){std::cerr<<"js: --check -e requires source\n";return 2;}source=argv[3];}else{std::ifstream in(argv[2],std::ios::binary);if(!in){std::cerr<<"js: cannot read "<<argv[2]<<"\n";return 2;}source.assign(std::istreambuf_iterator<char>(in),{});}
  jspp::Program program;jspp::Diagnostic diagnostic;if(!jspp::parse_source(source,program,diagnostic)){std::cerr<<"js: "<<jspp::format_diagnostic(diagnostic)<<"\n";return 2;}return 0;
 }
 bool dump=argc>=2&&std::string(argv[1])=="--dump-bytecode";int pos=dump?2:1;std::string source;
 if(pos<argc&&std::string(argv[pos])=="-e"){if(pos+1>=argc){std::cerr<<"js: -e requires source\n";return 2;}source=argv[pos+1];}else if(pos<argc){std::ifstream in(argv[pos],std::ios::binary);if(!in){std::cerr<<"js: cannot read "<<argv[pos]<<"\n";return 2;}source.assign(std::istreambuf_iterator<char>(in),{});}else{std::cerr<<"usage: js [-e source|file]\n";return 2;}
 if(dump){jspp::Program p;jspp::Diagnostic d;jspp::Bytecode b;std::string error;if(!jspp::parse_source(source,p,d)){std::cerr<<"js: "<<jspp::format_diagnostic(d)<<"\n";return 2;}if(!jspp::compile(p,b,error)){std::cerr<<"js: "<<error<<"\n";return 2;}std::cout<<jspp::disassemble(b);return 0;}
 auto*r=js_runtime_new();if(!r){std::cerr<<"js: cannot create runtime\n";return 2;}js_value*v=nullptr;const auto status=js_eval(r,source.c_str(),&v);if(status!=JS_STATUS_OK){std::cerr<<"js: "<<js_runtime_last_error(r)<<"\n";js_runtime_free(r);return 2;}switch(js_value_get_kind(v)){case JS_VALUE_UNDEFINED:std::cout<<"undefined\n";break;case JS_VALUE_NULL:std::cout<<"null\n";break;case JS_VALUE_BOOLEAN:{int x=0;js_value_get_boolean(v,&x);std::cout<<(x?"true":"false")<<"\n";break;}case JS_VALUE_NUMBER:{double x=0;js_value_get_number(v,&x);std::cout<<x<<"\n";break;}case JS_VALUE_STRING:{const char*s=nullptr;size_t n=0;js_value_get_string(v,&s,&n);std::cout.write(s,static_cast<std::streamsize>(n));std::cout<<"\n";break;}}js_value_free(r,v);js_runtime_free(r);return 0;
}
