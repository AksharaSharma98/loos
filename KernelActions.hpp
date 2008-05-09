/*
  KernelActions.hpp
  (c) 2008 Tod D. Romo


  Grossfield Lab
  Department of Biochemistry and Biophysics
  University of Rochester Medical School

*/



#if !defined(KERNELACTIONS_HPP)
#define KERNELACTIONS_HPP

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>

#include <string.h>

#include "KernelValue.hpp"
#include "KernelStack.hpp"


using namespace std;

namespace loos {
  class Action {
  protected:
    ValueStack *stack;

    int binComp(void) {
      Value v1 = stack->pop();
      Value v2 = stack->pop();
      return(compare(v1, v2));
    }

  public:
    Action() : stack(0) { }
    Action(ValueStack *stk) : stack(stk) { }

    void setStack(ValueStack* ptr) { stack=ptr; }
    

    virtual void execute(void) = 0;
    virtual ~Action() { }
  };


  class pushString : public Action {
    Value val;
  public:
    pushString(const string str) : val(str) { }
    void execute(void) { stack->push(val); }
  };

  class pushInt : public Action {
    Value val;
  public:
    pushInt(const int i) : val(i) { }
    void execute(void) { stack->push(val); }
  };

  class pushFloat : public Action {
    Value val;
  public:
    pushFloat(const float f) : val(f) { }
    void execute(void) { stack->push(val); }
  };

  class drop : public Action {
  public:
    drop() { }
    void execute(void) { stack->drop(); }
  };

  class dup : public Action {
  public:
    dup() { }
    void execute(void) { stack->dup(); }
  };

  class equals : public Action {
  public:
    equals() { }
    void execute(void) {
      Value v(binComp() == 0);
      stack->push(v);
    }
  };

  class lessThan : public Action {
  public:
    lessThan() { }
    void execute(void) {
      Value v(binComp() < 0);
      stack->push(v);
    }
  };

  class lessThanEquals : public Action {
  public:
    lessThanEquals() { }
    void execute(void) {
      Value v(binComp() <= 0);
      stack->push(v);
    }
  };

  class greaterThan : public Action {
  public:
    greaterThan() { }
    void execute(void) {
      Value v(binComp() > 0);
      stack->push(v);
    }
  };

  class greaterThanEquals : public Action {
  public:
    greaterThanEquals() { }
    void execute(void) {
      Value v(binComp() >= 0);
      stack->push(v);
    }
  };


};



#endif

