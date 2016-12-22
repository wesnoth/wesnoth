import os.path

def CheckIEEE754(context):
    context.Message("Checking if floating point numbers are in the IEEE 754 format... ")
    test_file = open(os.path.join("src", "compile_time_tests", "ieee_754.cpp"))
    ret, _ = context.TryRun(test_file.read(), ".cpp")
    context.Result(ret)
    return ret

config_checks = { "CheckIEEE754" : CheckIEEE754 }
