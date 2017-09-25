from os.path import normpath

def exists():
    return True

def make_incflags(paths, RDirs):
    result = []
    for path in paths:
       if not str(path).startswith("#") and normpath(str(path)) != '/usr/include':
           for rdir in RDirs((path,)):
               result.append("-isystem")
               result.append(str(rdir))
       else:
           for rdir in RDirs((path,)):
               result.append("-I" + str(rdir))
    return result

def generate(env):
    if "gcc" in env["TOOLS"]:
        env["make_incflags"] = make_incflags
        env["INCPREFIX"] = ""
        env["_CPPINCFLAGS"] = "$( ${_concat(INCPREFIX, CPPPATH, INCSUFFIX, __env__, lambda x : make_incflags(x, RDirs), TARGET, SOURCE)} $)"
