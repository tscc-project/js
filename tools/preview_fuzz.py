#!/usr/bin/env python3
import random, subprocess, sys

js=sys.argv[1]
random.seed(0x4A535050)
seeds=["1+2;","function f(x){return x+1;}f(2);","({x:[1,2]}).x[0];","try{throw 1;}catch(e){e;}"]
alphabet="{}[]();,+-*/=!<>?:.'\"abcdefghijklmnopqrstuvwxyz0123456789 "
for index in range(400):
    source=list(random.choice(seeds))
    for _ in range(1+random.randrange(5)):
        action=random.randrange(3);position=random.randrange(len(source)+1)
        if action==0:source.insert(position,random.choice(alphabet))
        elif action==1 and source:source.pop(min(position,len(source)-1))
        elif source:source[min(position,len(source)-1)]=random.choice(alphabet)
    try:result=subprocess.run([js,"-e","".join(source)],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,timeout=1)
    except subprocess.TimeoutExpired:raise SystemExit(f"mutation {index} timed out")
    if result.returncode not in (0,2):raise SystemExit(f"mutation {index} exited {result.returncode}: {''.join(source)!r}")
print("JS++ deterministic malformed-input fuzz passed: 400 mutations")
