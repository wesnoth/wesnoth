#ifndef DISABLE_4786_HPP_INCLUDED
#define DISABLE_4786_HPP_INCLUDED

#ifdef _MSC_VER

//disable the warning to let us know about 'this' being used in
//initializer list, since it's a common thing to want to do
//for callbacks, and there is no other way to easily workaround the warning
#pragma warning(disable:4355)

//disable the warnings for long template names
#pragma warning(disable:4503)
#pragma warning(disable:4786)

//the following code causes the incredibly irritating warning 4786 to really
//be muted in Visual C++ 6. No-one seems to know *why* it works (possibly not even Microsoft)
//but it does. So don't ask, and just leave it there.

class warning4786WorkAround {
public:
warning4786WorkAround() {}
};

static warning4786WorkAround VariableThatHacksWarning4786IntoBeingMutedForSomeUnknownReason;

//put the mathematical functions where they belong: in the std namespace
//it is necessary for VC6 at least
#include <cmath>
namespace std {
  using ::floor;
  using ::sqrt;
}

#endif

#endif
