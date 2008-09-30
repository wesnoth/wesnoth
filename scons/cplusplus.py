# vi: syntax=python:et:ts=4
from config_check_utils import *

def CheckCPlusPlus(context, gcc_version = None):
    message = "Checking whether C++ compiler works "
    test_program = """
    #include <iostream>
    int main()
    {
    std::cout << "Hello, world\\n";
    }
    """
    if gcc_version and "gcc" in context.env["TOOLS"]:
        message += "(g++ version >= %s required)" % gcc_version
        import operator
        version = gcc_version.split(".", 3)
        version = map(int, version)
        version = map(lambda x,y: x or y, version, (0,0,0))
        multipliers = (10000, 100, 1)
        version_num = sum(map(operator.mul, version, multipliers))
        test_program += """
        #define GCC_VERSION (__GNUC__ * 10000 \\
                           + __GNUC_MINOR__ * 100 \\
                           + __GNUC_PATCHLEVEL__)
        #if GCC_VERSION < %d
        #error Compiler version is too old!
        #endif
        \n""" % version_num
    message += "... "
    context.Message(message)
    if context.TryBuild(context.env.Program, test_program, ".cpp") == 1 and context.lastTarget.get_contents() != "":
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

config_checks = { "CheckCPlusPlus" : CheckCPlusPlus }
