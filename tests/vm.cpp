#include "Bytecode.h"
#include "Frontend.h"
#include "js.h"
#include <cstdlib>
#include <iostream>
#include <string>
static void check(bool x,const char*m){if(!x){std::cerr<<m<<'\n';std::exit(1);}}
static double number(js_runtime*r,const char*source){js_value*v=nullptr;check(js_eval(r,source,&v)==JS_STATUS_OK,"numeric evaluation");double n=0;check(js_value_get_number(v,&n),"numeric result");js_value_free(r,v);return n;}
int main(){
 jspp::Program p;jspp::Diagnostic d;check(jspp::parse_source("const x=2+3*4;x>=14;",p,d),"parse");jspp::Bytecode b;std::string e;check(jspp::compile(p,b,e),"compile");auto listing=jspp::disassemble(b);check(listing.find("multiply")!=std::string::npos&&listing.find("declare x")!=std::string::npos,"bytecode inspection");
 auto*r=js_runtime_new();js_value*v=nullptr;check(js_eval(r,"let x=6;(x*7)===42;",&v)==JS_STATUS_OK,"execute");int yes=0;check(js_value_get_boolean(v,&yes)&&yes,"boolean result");js_value_free(r,v);
 check(number(r,"let sum=0;for(let i=0;i<8;i=i+1){if(i===2)continue;if(i===6)break;sum=sum+i;}sum;")==13,"control flow result");
 check(number(r,"function outer(x){return function(y){x=x+1;return x+y;};}const f=outer(39);f(1);f(1);")==42,"closure capture result");
 check(number(r,"function fact(n){if(n<=1)return 1;return n*fact(n-1);}fact(6);")==720,"recursive result");
 check(number(r,"const add=(a,b)=>a+b;add(20,22);")==42,"arrow result");
 check(number(r,"const o={answer:40};o.answer=o.answer+2;o['answer'];")==42,"object property result");
 check(number(r,"const a=[10,20];a[2]=12;a[0]+a[1]+a[2];")==42,"array property result");
 check(number(r,"const a={x:1};const b=a;b.x=42;a.x;")==42,"object aliasing result");
 check(number(r,"const x=42;({x}).x;")==42,"object shorthand result");
 check(number(r,"[1,,3].length;")==3,"array hole length result");
 check(number(r,"const o={x:40,add(n){this.x=this.x+n;return this.x;}};o.add(2);")==42,"method receiver result");
 check(number(r,"function Answer(n){this.value=n;}Answer.prototype.read=function(){return this.value;};const a=new Answer(42);a.read();")==42,"constructor prototype result");
 check(number(r,"let x=1;{let x=2;}x;")==1,"block scope result");
 check(js_eval(r,"const fixed=1;fixed=2;",&v)==JS_STATUS_RUNTIME_ERROR,"const assignment accepted");check(js_eval(r,"missing+1",&v)==JS_STATUS_RUNTIME_ERROR,"missing identifier accepted");check(js_eval(r,"1+true",&v)==JS_STATUS_RUNTIME_ERROR,"mixed arithmetic accepted");check(js_eval(r,"let x=1;let x=2;",&v)==JS_STATUS_SYNTAX_ERROR,"duplicate binding accepted");
 js_runtime_free(r);std::cout<<"JS++ functions/closures VM passed\n";
}
