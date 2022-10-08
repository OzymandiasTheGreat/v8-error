#ifndef hlp_CallInvokerGetter_h
#define hlp_CallInvokerGetter_h

#include <memory>
#include <ReactCommon/CallInvoker.h>

namespace holepunch {

using namespace facebook;

// we need this since invoking a method on invalidated bridge will crash

struct CallInvokerGetter {
  virtual bool isValid() {
    return false;
  };
  virtual std::shared_ptr<react::CallInvoker> get() {
    return NULL;
  };
};

}

#endif /* hlp_CallInvokerGetter_h */
