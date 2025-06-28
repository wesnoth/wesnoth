# vi: syntax=python:et:ts=4
import os
import os.path
from SCons.Script import *
from config_check_utils import *
from os import environ
from SCons.Util import PrependPath

def CheckSDL3(context, require_version):
    version = require_version.split(".", 2)
    major_version = version[0]
    minor_version = version[1]
    patchlevel = version[2]
    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])
    sdldir = context.env.get("sdldir", "")
    context.Message("Checking for Simple DirectMedia Layer library version >= %s.%s.%s... " % (major_version, minor_version, patchlevel))

    env = context.env
    if sdldir:
        env["ENV"]["PATH"] = PrependPath(environ["PATH"], join(sdldir, "bin"))
        env["ENV"]["PKG_CONFIG_PATH"] = PrependPath(environ.get("PKG_CONFIG_PATH", ""), join(sdldir, "lib/pkgconfig"))

    if env["PLATFORM"] != "win32" or sys.platform == "msys":
        for foo_config in [
            "pkg-config --cflags --libs $PKG_CONFIG_FLAGS sdl3",
            "sdl3-config --cflags --libs"
            ]:
            try:
                env.ParseConfig(foo_config)
            except OSError:
                pass
            else:
                break
    else:
        if sdldir:
            env.AppendUnique(CPPPATH = [os.path.join(sdldir, "include/SDL3")], LIBPATH = [os.path.join(sdldir, "lib")])
        env.AppendUnique(CCFLAGS = ["-D_GNU_SOURCE"])
        env.AppendUnique(LIBS = Split("mingw32 SDL3main SDL3"))
        env.AppendUnique(LINKFLAGS = ["-mwindows", "-fstack-protector"])

    cpp_file = File("src/conftests/sdl3.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl3.cpp"

    with open(cpp_file, 'r') as file:
        test_program = file.read()

        if context.TryLink(test_program, ".cpp"):
            context.Result("yes")
            return True
        else:
            context.Result("no")
            restore_env(context.env, backup)
            return False

def CheckSDL3Image(context):
    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])
    context.Message("Checking for SDL3_image library... ")
    context.env.AppendUnique(LIBS = ["SDL3_image"])

    cpp_file = File("src/conftests/sdl3_image.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl3_image.cpp"

    with open(cpp_file, 'r') as file:
        test_program = file.read()

        if context.TryLink(test_program, ".cpp"):
            context.Result("yes")
            return True
        else:
            context.Result("no")
            restore_env(context.env, backup)
            return False

def CheckSDL3Mixer(context):
    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])
    context.Message("Checking for SDL3_mixer library... ")
    context.env.AppendUnique(LIBS = ["SDL3_mixer"])

    cpp_file = File("src/conftests/sdl3_mixer.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl3_mixer.cpp"

    with open(cpp_file, 'r') as file:
        test_program = file.read()

        if context.TryLink(test_program, ".cpp"):
            context.Result("yes")
            return True
        else:
            context.Result("no")
            restore_env(context.env, backup)
            return False

def CheckOgg(context):
    context.env["ENV"]["SDL_AUDIODRIVER"] = "dummy"

    cpp_file = File("src/conftests/sdl3_audio.cpp").rfile().get_contents().decode()

    # file1 (absolute path) works most places
    # file2 (relative path) is required for msys2 on windows
    # both need to be executed since just checking for the file's existence isn't enough
    # python in the msys2 shell will find the absolute path, but the actual windows executable doesn't understand absolute unix-style paths
    ogg_file1 = File("data/core/music/main_menu.ogg").rfile().abspath
    ogg_file2 = "data/core/music/main_menu.ogg"

    context.Message("Checking for audio support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True

    (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+ogg_file1+"\""), ".cpp")

    if result:
        context.Result("yes")
        return True
    else:
        (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+ogg_file2+"\""), ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

def CheckPNG(context):
    cpp_file = File("src/conftests/sdl3_png.cpp").rfile().get_contents().decode()

    img_file1 = File("data/core/images/scons_conftest_images/end-n.png").rfile().abspath
    img_file2 = "data/core/images/scons_conftest_images/end-n.png"

    context.Message("Checking for PNG support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True

    (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+img_file1+"\""), ".cpp")

    if result:
        context.Result("yes")
        return True
    else:
        (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+img_file2+"\""), ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

def CheckWebP(context):
    cpp_file = File("src/conftests/sdl3_webp.cpp").rfile().get_contents().decode()

    img_file1 = File("data/core/images/scons_conftest_images/end-n.webp").rfile().abspath
    img_file2 = "data/core/images/scons_conftest_images/end-n.webp"

    context.Message("Checking for WEBP support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True

    (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+img_file1+"\""), ".cpp")

    if result:
        context.Result("yes")
        return True
    else:
        (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+img_file2+"\""), ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

def CheckJPG(context):
    cpp_file = File("src/conftests/sdl3_jpg.cpp").rfile().get_contents().decode()

    img_file1 = File("data/core/images/scons_conftest_images/end-n.jpg").rfile().abspath
    img_file2 = "data/core/images/scons_conftest_images/end-n.jpg"

    context.Message("Checking for JPG support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True

    (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+img_file1+"\""), ".cpp")

    if result:
        context.Result("yes")
        return True
    else:
        (result, output) = context.TryRun(cpp_file.replace("argv[1]", "\""+img_file2+"\""), ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

config_checks = { 'CheckSDL3Image' : CheckSDL3Image,
                  'CheckSDL3Mixer' : CheckSDL3Mixer,
                  'CheckSDL3': CheckSDL3,
                  'CheckOgg' : CheckOgg,
                  'CheckPNG' : CheckPNG,
                  'CheckJPG' : CheckJPG,
                  'CheckWebP' : CheckWebP }
