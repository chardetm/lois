// automatically test the LOIS library.

#include <stdlib.h>
#include <sys/time.h>

#include "../include/loisextra.h"
#include "../include/lois-weak.h"
#include "../include/lois-automaton.h"

#include <iostream>
using namespace std;
using namespace lois;

long long getVa() {
  struct timeval tval;
  gettimeofday(&tval, NULL);
  return tval.tv_sec * 1000000 + tval.tv_usec;
  }

long long lasttime;

void showTimeElapsed() {
  long long curtime = getVa();
  
  if(curtime - lasttime < 100000) 
    cout << "time elapsed: " << (curtime-lasttime) << " \u03bcs" << endl;

  else
    printf("time elapsed: %.6f s\n", (curtime-lasttime)/1000000.0);

  lasttime = curtime;
  }

#define MAYBE 3
#define ALWAYS 1
#define NEVER 2

string modenames[4] = {"inconsistent", "always", "never", "maybe"};

void testSat(int mode, rbool phi) {
  int cnt = 0;
  If(phi) cnt ++;
  If(!phi) cnt += 2;
  if(cnt == mode) cout << "passed: ";
  else cout << "failed: ";
  cout << modenames[cnt] << " " << phi << endl;
  if(cnt != mode) exit(1);
  }

/*
inline lset& operator += (lset& x, const lset& y) {
  return x += elem(y);
  } */

/*
inline lset& operator += (lset& x, const rset y) {
  return x += elem(y);
  } */

template<class State, class Symbol> void minimalize1(lsetof<State> Q, lsetof<State> F, lsetof<transition<State,Symbol>> delta, lsetof<Symbol> alph) {
  cout << "Q = " << Q << endl;
  cout << "F = " << F << endl;
  cout << "delta = " << delta << endl;
  
  lsetof<State> NF = Q &~ F;
  cout << "NF = " << NF << endl << endl;

  lsetof<lpair<State,State>> eq = (F * F) | (NF * NF);
  
  int it = 0;
  
  lbool cont = true;
  
  While(cont) {
    cout << "eq = " << eq << endl << endl;
    
    lsetof<lpair<State,State>> neq;
    
    for(State q1: Q) for(State q2: Q) If(memberof(make_lpair(q1,q2), eq)) {
      lbool b = true;
      for(Symbol& x: alph) for(auto& t1: delta) for(auto& t2: delta)
        If(t1.src == q1 && t1.symbol == x && t2.src == q2 && t2.symbol == x) {
          If(!memberof(make_lpair(t1.tgt, t2.tgt), eq)) {
            b = ffalse;
            }
          }
      If(b) neq += make_lpair(q1, q2);
      }
    
    If(eq == neq) cont = ffalse;
    eq = neq; it++;
    }

  cout << "number of iterations: " << it << endl << endl;
  
  lsetof<lsetof<State>> classes;
  
  for(State& a: Q) {
    lsetof<State> t;
    for(State& b: Q) If(memberof(make_lpair(a,b), eq)) 
      t += b;
    classes += t;
    }
  
  cout << "unoptimized classes: " << classes << endl;
  classes = optimize(classes);

  cout << "classes: " << classes << endl;  
  }

template<class State, class Symbol> void minimalize2(lsetof<State> Q, lsetof<State> F, lsetof<transition<State,Symbol>> delta, lsetof<Symbol> alph) {
  cout << "Q = " << Q << endl;
  cout << "F = " << F << endl;
  cout << "delta = " << delta << endl;
  
  lsetof<lsetof<State>> classes;
  classes += F;
  classes += Q &~ F;
  
  int it = 0;
  
  lbool cont = true;
  
  While(cont) {
    lsetof<lsetof<State>> nclasses;
    cout << "classes = " << classes << endl << endl;
    cont = ffalse;
    
    for(auto& EC: classes) {
      for(State& q1: EC) {
        lset EC1;
        for(State& q2: EC) {
          lbool b = true;
          for(auto& t1: delta) for(auto& t2: delta) 
            If(t1.src == q1 && t2.src == q2 && t1.symbol == t2.symbol)
            If(EXISTS(c, classes, memberof(t1.tgt, c) && !memberof(t2.tgt, c)))
              b &= ffalse;
          Ife(b) EC1 += q2;
          else cont = ftrue;
          }
        nclasses += EC1;
        }
      }

    classes = optimize(nclasses); it++;
    }

  cout << "number of iterations: " << it << endl << endl;
  
  cout << "classes: " << classes << endl;  
  }

elem eltuple(std::initializer_list<term> l) {
  return elof(lvector<term>(l));
  }

namespace lois {
  extern int simplifyVerbosity;
  }

int main() {
  lasttime = getVa();
  initLois();

  sym.neq = "≠";

// this will not output anything
// and thus work more effectively
#define solverNamed(x,y) y

// avoid indentation to create a smaller incremental0.smt
//  iindent.i = iunindent.i = 0;

//solver = 
//  solverBasic() ||
//  solverCompare({
//      solverNamed("internal", solverExhaustive(500, false) || solverExhaustive(2000000000, true)),  
//      solverNamed("z3", solverIncremental("z3-*/bin/z3 -smt2 -in")),
      // solverNamed("crash", solverIncremental("echo crash")),
        // solverNamed("cvc4", solverIncremental("cvc4 --lang smt --incremental --finite-model-find")),
     // solverNamed("cvc4", solverIncremental("cvc4 --lang smt --incremental"))
//      }) ||
//  solverCrash();

  Domain dA("Real");
  lsetof<term> A = dA.getSet();
  
  typedef elem state;
  typedef term symbol;
  
  lsetof<state> Q;
  lsetof<state> F;
  lsetof<transition<state,symbol>> delta;
  
//   for (elem x:A) for (elem y:A) {
//     elem u=make_lpair(x,y);
//     If (x==y)
//              R+= make_lpair(x,y);
// 	If (x!=y)
//       R+=eltuple({x,x,y});
// //    R += u;
//   }
//   cout << "R= " << R << endl;
    

  lsetof<symbol> alph = A;

  Q += elof(0);
  for(symbol& a: A) Q += eltuple({a});
  for(symbol& a: A) for(symbol& b: A) Q += eltuple({a, b});
  for(symbol& a: A) for(symbol& b: A) for(symbol& c: A) Q += eltuple({a, b, c});
  Q += elof(4);
  
  for(symbol& a: A) for(symbol& b: A) for(symbol& c: A) 
    If(a==b || a==c || b==c) F += eltuple({a,b,c});
    
  for(symbol& x: A) {
    delta += make_transition(elof(0),x,eltuple({x}));

    for(symbol& a: A)
      delta += make_transition(eltuple({a}),x,eltuple({a,x}));

    for(symbol& a: A) for(symbol& b: A)
      delta += make_transition(eltuple({a,b}),x,eltuple({a,b,x}));

    for(symbol& a: A) for(symbol& b: A) for(symbol& c: A)
      delta += make_transition(eltuple({a,b,c}),x,elof(4));

    delta += make_transition(elof(4),x,elof(4));
    }

  /* Q += 0; Q += 1; Q += 2; Q += 3; F += 0; F += 2; F += 1;
  for(elem x: A) delta += eltuple({elof(0), x, elof(1)});
  for(elem x: A) delta += eltuple({elof(1), x, elof(2)});
  for(elem x: A) delta += eltuple({elof(2), x, elof(3)});
  for(elem x: A) delta += eltuple({elof(3), x, elof(0)}); */

  minimalize1(Q, F, delta, alph);
  showTimeElapsed();

  minimalize2(Q, F, delta, alph);
  showTimeElapsed();

  return 0;
  }
