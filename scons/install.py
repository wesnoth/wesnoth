# vi: syntax=python:et:ts=4
from SCons.Script import *
import shutil, os
from subprocess import call

def InstallFilteredHook(target, source, env):
    CopyFilter = env["copy_filter"]
    if type(target) == type([]):
        target = target[0]
    target = str(target)
    if type(source) == type([]):
        map(lambda f: InstallFilteredHook(target, f, env), source)
    elif os.path.isdir(str(source)):
        if CopyFilter(source):
            target = os.path.join(target, os.path.basename(str(source))) 
            if not os.path.exists(target):
                 if env["verbose"]:
                      print "Make directory", target
                 os.makedirs(target)
            map(lambda f: InstallFilteredHook(target, os.path.join(str(source), f), env), os.listdir(str(source)))
    elif CopyFilter(source):
        if (env["gui"] == "tiny") and (source.endswith("jpg") or source.endswith("png")):
             image_info = Popen(["identify", "-verbose", source], stdout = PIPE).communicate()[0]
             target = os.path.join(target, os.path.basename(source))
             if "Alpha: " in image_info:
                 command = "convert -filter point -resize %s %s %s"
             else:
                 command = "convert -resize %s %s %s"
             for (large, small) in (("1024x768","320x240"),
                                    ("640x480","240x180"),
                                    ("205x205","80x80")):
                if ("Geometry: " + large) in image_info:
                    command = command % (small, source, target)
                    break
             else:
                    command = command % ("50%", source, target)
             if env["verbose"]:
                print command
             call(Split(command))
             return None
        # Just copy non-images, and images if tinygui is off
        if env["verbose"]:
             print "cp %s %s" % (str(source), target)
        shutil.copy2(str(source), target)
    return None

def InstallWithSuffix(env, target, source):
    if not source:
        return source
    return env.InstallAs(os.path.join(target, source[0].name + env["program_suffix"]), source)

def generate(env):
    #env.AddMethod(InstallWithSuffix)
    from SCons.Script.SConscript import SConsEnvironment
    SConsEnvironment.InstallWithSuffix = InstallWithSuffix

    env.Append(BUILDERS={'InstallFiltered':Builder(action=InstallFilteredHook)})

def exists():
    return True
