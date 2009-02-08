import re, os, safe

whitelisted = ["ai", "collections", "functools", "heapq", "math", "Queue", "re", "sets", "string", "threading", "time", "wail", "wesnoth"]
rex = re.compile(r"^import\s+(.*)", re.M)
modules = {}

def include(matchob):
    """
    Regular expression callback. Handles a single import statement, returning
    the included code.
    """
    names = [x.strip() for x in matchob.group(1).split(",")]
    r = ""
    for name in names:
        alias = None
        if " as " in name:
            (name, ignored, alias) = name.split(' ')
        if name in whitelisted:
            if alias:
                modules[alias] = __import__(name)
            else:
                modules[name] = __import__(name)
            continue
        for path in pathes:
            includefile = os.path.join(path, name)
            try:
                code = parse_file(includefile + ".py")
                break
            except IOError:
                pass
        else:
            raise safe.SafeException("Could not import '%s'." % name)

        r += code

    return r

def parse_file(name):
    """
    Simple pre-parsing of scripts, all it does is allow importing other scripts.
    """
    abspath = os.path.abspath(name)
    if abspath in already: return ""
    already[abspath] = 1
    code = file(abspath).read().replace(chr(13), "")
    code = rex.sub(include, code)
    return code

# If you want to disable safe python, use this instead:
#
# def parse(name): return open(name).read(), {}
def parse(name):
    global already, modules
    already = {}
    modules = {}
    return parse_file(name), modules

