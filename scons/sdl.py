# vi: syntax=python:et:ts=4
import os
from SCons.Script import *
from config_check_utils import *

def CheckSDL(context, sdl_lib = "SDL", require_version = None):
    if require_version:
        version = require_version.split(".", 2)
        major_version = int(version[0])
        minor_version = int(version[1])
        try:
            patchlevel    = int(version[2])
        except (ValueError, IndexError):
            patch_level = 0

    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])

    sdldir = context.env.get("sdldir", "")
    if sdl_lib == "SDL": 
        if require_version:
            context.Message("Checking for Simple DirectMedia Layer library version >= %d.%d.%d... " % (major_version, minor_version, patchlevel))
        else:
            context.Message("Checking for Simple DirectMedia Layer library... ")
        env = context.env
        if sdldir:
            env.AppendUnique(CPPPATH = [os.path.join(sdldir, "include/SDL")], LIBPATH = [os.path.join(sdldir, "lib")])
        else:
            for foo_config in [
                "pkg-config --cflags --libs sdl",
                "sdl-config --cflags --libs"
                ]:
                try:
                    env.ParseConfig(foo_config)
                except OSError:
                    pass
                else:
                    break
        if env["PLATFORM"] == "win32":
            env.AppendUnique(CCFLAGS = ["-D_GNU_SOURCE"])
            env.AppendUnique(LIBS = Split("mingw32 SDLmain SDL"))
            env.AppendUnique(LINKFLAGS = ["-mwindows"])
    else:
        if require_version:
            context.Message("Checking for %s library version >= %d.%d.%d... " % (sdl_lib, major_version, minor_version, patchlevel))
        else:
            context.Message("Checking for %s library... " % sdl_lib)
        context.env.AppendUnique(LIBS = [sdl_lib])
    test_program = """
        #include <%s.h> 
        \n""" % sdl_lib
    if require_version:
        test_program += "#if SDL_VERSIONNUM(%s, %s, %s) < SDL_VERSIONNUM(%d, %d, %d)\n#error Library is too old!\n#endif\n" % \
            (sdl_lib.upper() + "_MAJOR_VERSION", \
             sdl_lib.upper() + "_MINOR_VERSION", \
             sdl_lib.upper() + "_PATCHLEVEL", \
             major_version, minor_version, patchlevel)
    test_program += """
        int main(int argc, char** argv)
        {
        }
        \n"""
    if context.TryLink(test_program, ".c"):
        context.Result("yes")
        return True
    else:
        context.Result("no")
        restore_env(context.env, backup)
        return False

def CheckOgg(context):
    test_program = '''
    #include <SDL_mixer.h>
    #include <stdlib.h>

    int main(int argc, char **argv)
    {
        Mix_Music* music = Mix_LoadMUS("$TESTFILE");
        if (music == NULL) {
            exit(1);
        }
        exit(0);
    }
\n
'''
    test_program = context.env.Clone(TESTFILE = File("data/core/music/main_menu.ogg").rfile().abspath).subst(test_program)
    #context.env.AppendUnique(LIBS = "SDL_mixer")
    context.Message("Checking for Ogg Vorbis support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True
    (result, output) = context.TryRun(test_program, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

def CheckPNG(context):
    test_program = '''
    #include <SDL_image.h>
    #include <stdlib.h>

    int main(int argc, char **argv)
    {
            SDL_RWops *src;
            char *testimage = "$TESTFILE";

            src = SDL_RWFromFile(testimage, "rb");
            if (src == NULL) {
                    exit(2);
            }
            exit(!IMG_isPNG(src));
    }
\n
'''
    test_program = context.env.Clone(TESTFILE = File("images/buttons/button_normal/button_H22-pressed.png").rfile().abspath).subst(test_program)
    context.Message("Checking for PNG support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True
    (result, output) = context.TryRun(test_program, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

def CheckJPG(context):
    test_program = '''
    #include <SDL_image.h>
    #include <stdlib.h>

    int main(int argc, char **argv)
    {
            SDL_RWops *src;
            char *testimage = "$TESTFILE";

            src = SDL_RWFromFile(testimage, "rb");
            if (src == NULL) {
                    exit(2);
            }
            exit(!IMG_isJPG(src));
    }
\n
'''
    test_program = context.env.Clone(TESTFILE = File("data/core/images/maps/background.jpg").rfile().abspath).subst(test_program)
    context.Message("Checking for JPG support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True
    (result, output) = context.TryRun(test_program, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        context.Result("no")
        return False

config_checks = { 'CheckSDL' : CheckSDL,
                  'CheckOgg' : CheckOgg,
                  'CheckPNG' : CheckPNG,
                  'CheckJPG' : CheckJPG }
