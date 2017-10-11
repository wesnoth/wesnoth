import os.path
from SCons.Script import File

def CheckIEEE754(context):
    context.Message("Checking if floating point numbers are in the IEEE 754 format... ")
    test_file = File(os.path.join("src", "compile_time_tests", "ieee_754.cpp")).get_contents().decode("utf-8")
    ret, _ = context.TryRun(test_file, ".cpp")
    context.Result(ret)
    return ret

config_checks = { "CheckIEEE754" : CheckIEEE754 }
