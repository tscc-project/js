#include "Frontend.h"
#include <cstdlib>
#include <iostream>
using namespace jspp;static void check(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
int main(){Program p;Diagnostic d;check(parse_source("const x=1+2*3; x>=7;",p,d),"valid source rejected");check(p.statements.size()==2,"statement count");check(p.statements[0].expression->kind==ExprKind::Binary,"expression tree missing");Program bad;check(!parse_source("let = 1",bad,d)&&d.line==1&&d.column==5,"bad binding accepted");Program eof;check(!parse_source("(1+2",eof,d),"missing delimiter accepted");Program unsupported;check(!parse_source("if(true) 1;",unsupported,d),"unsupported statement accepted");std::cout<<"JS++ frontend passed\n";}
