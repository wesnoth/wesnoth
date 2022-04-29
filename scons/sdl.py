# vi: syntax=python:et:ts=4
import os
import os.path
from SCons.Script import *
from config_check_utils import *
from os import environ
from SCons.Util import PrependPath

def CheckSDL2(context, require_version):
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
            "pkg-config --cflags --libs $PKG_CONFIG_FLAGS sdl2",
            "sdl2-config --cflags --libs"
            ]:
            try:
                env.ParseConfig(foo_config)
            except OSError:
                pass
            else:
                break
    else:
        if sdldir:
            env.AppendUnique(CPPPATH = [os.path.join(sdldir, "include/SDL2")], LIBPATH = [os.path.join(sdldir, "lib")])
        env.AppendUnique(CCFLAGS = ["-D_GNU_SOURCE"])
        env.AppendUnique(LIBS = Split("mingw32 SDL2main SDL2"))
        env.AppendUnique(LINKFLAGS = ["-mwindows"])

    cpp_file = File("src/conftests/sdl2.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl2.cpp"

    with open(cpp_file, 'r') as file:
        test_program = file.read().replace("argv[1]", "\""+major_version+"\"").replace("argv[2]", "\""+minor_version+"\"").replace("argv[3]", "\""+patchlevel+"\"")

        if context.TryLink(test_program, ".cpp"):
            context.Result("yes")
            return True
        else:
            context.Result("no")
            restore_env(context.env, backup)
            return False

def CheckSDL2Image(context):
    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])
    context.Message("Checking for SDL2_image library... ")
    context.env.AppendUnique(LIBS = ["SDL2_image"])

    cpp_file = File("src/conftests/sdl2_image.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl2_image.cpp"

    with open(cpp_file, 'r') as file:
        test_program = file.read()

        if context.TryLink(test_program, ".cpp"):
            context.Result("yes")
            return True
        else:
            context.Result("no")
            restore_env(context.env, backup)
            return False

def CheckSDL2Mixer(context):
    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])
    context.Message("Checking for SDL2_mixer library... ")
    context.env.AppendUnique(LIBS = ["SDL2_mixer"])

    cpp_file = File("src/conftests/sdl2_mixer.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl2_mixer.cpp"

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

    cpp_file = File("src/conftests/sdl2_audio.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl2_audio.cpp"

    ogg_file = File("data/core/music/main_menu.ogg").rfile().abspath
    if not os.path.isfile(ogg_file):
        ogg_file = "data/core/music/main_menu.ogg"

    with open(cpp_file, 'r') as file:
        test_program = file.read().replace("argv[1]", "\""+ogg_file+"\"")

        context.Message("Checking for audio support in SDL... ")
        if context.env["host"]:
            context.Result("n/a (cross-compile)")
            return True
        (result, output) = context.TryRun(test_program, ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

def CheckPNG(context):
    cpp_file = File("src/conftests/sdl2_png.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl2_png.cpp"

    img_file = File("data/core/images/scons_conftest_images/end-n.png").rfile().abspath
    if not os.path.isfile(img_file):
        img_file = "data/core/images/scons_conftest_images/end-n.png"

    with open(cpp_file, 'r') as file:
        test_program = file.read().replace("argv[1]", "\""+img_file+"\"")

        context.Message("Checking for PNG support in SDL... ")
        if context.env["host"]:
            context.Result("n/a (cross-compile)")
            return True
        (result, output) = context.TryRun(test_program, ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

def CheckWebP(context):
    cpp_file = File("src/conftests/sdl2_webp.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl2_webp.cpp"

    img_file = File("data/core/images/scons_conftest_images/end-n.webp").rfile().abspath
    if not os.path.isfile(img_file):
        img_file = "data/core/images/scons_conftest_images/end-n.webp"

    with open(cpp_file, 'r') as file:
        test_program = file.read().replace("argv[1]", "\""+img_file+"\"")

        context.Message("Checking for WEBP support in SDL... ")
        if context.env["host"]:
            context.Result("n/a (cross-compile)")
            return True
        (result, output) = context.TryRun(test_program, ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

def CheckJPG(context):
    cpp_file = File("src/conftests/sdl2_jpg.cpp").rfile().abspath
    if not os.path.isfile(cpp_file):
        cpp_file = "src/conftests/sdl2_jpg.cpp"

    img_file = File("data/core/images/scons_conftest_images/end-n.jpg").rfile().abspath
    if not os.path.isfile(img_file):
        img_file = "data/core/images/scons_conftest_images/end-n.jpg"

    with open(cpp_file, 'r') as file:
        test_program = file.read().replace("argv[1]", "\""+img_file+"\"")

        context.Message("Checking for JPG support in SDL... ")
        if context.env["host"]:
            context.Result("n/a (cross-compile)")
            return True
        (result, output) = context.TryRun(test_program, ".cpp")
        if result:
            context.Result("yes")
            return True
        else:
            context.Result("no")
            return False

config_checks = { 'CheckSDL2Image' : CheckSDL2Image,
                  'CheckSDL2Mixer' : CheckSDL2Mixer,
                  'CheckSDL2': CheckSDL2,
                  'CheckOgg' : CheckOgg,
                  'CheckPNG' : CheckPNG,
                  'CheckJPG' : CheckJPG,
                  'CheckWebP' : CheckWebP }
