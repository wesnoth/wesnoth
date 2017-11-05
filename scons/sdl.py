# vi: syntax=python:et:ts=4
import os
from SCons.Script import *
from config_check_utils import *
from os import environ
from SCons.Util import PrependPath

def CheckSDL(context, sdl_lib = "SDL", require_version = None, header_file = None):
    if require_version:
        version = require_version.split(".", 2)
        major_version = int(version[0])
        minor_version = int(version[1])
        try:
            patchlevel    = int(version[2])
        except (ValueError, IndexError):
            patch_level = 0

    if header_file:
        sdl_header = header_file
    else:
        sdl_header = sdl_lib

    backup = backup_env(context.env, ["CPPPATH", "LIBPATH", "LIBS"])

    sdldir = context.env.get("sdldir", "")
    if sdl_lib == "SDL": 
        if require_version:
            context.Message("Checking for Simple DirectMedia Layer library version >= %d.%d.%d... " % (major_version, minor_version, patchlevel))
        else:
            context.Message("Checking for Simple DirectMedia Layer library... ")
        if major_version == 2:
            sdl_config_name = "sdl2-config"
            sdl_include_dir = "include/SDL2"
            sdl_lib_name = "SDL2"
            sdl_lib_name_pkgconfig = "sdl2"
            sdlmain_name = "SDL2main"
        else:
            sdl_config_name = "sdl-config"
            sdl_include_dir = "include/SDL"
            sdl_lib_name = "SDL"
            sdl_lib_name_pkgconfig = "sdl"
            sdlmain_name = "SDLmain"
        env = context.env
        if sdldir:
            env["ENV"]["PATH"] = PrependPath(environ["PATH"], join(sdldir, "bin"))
            env["ENV"]["PKG_CONFIG_PATH"] = PrependPath(environ.get("PKG_CONFIG_PATH", ""), join(sdldir, "lib/pkgconfig"))
        if env["PLATFORM"] != "win32":
            for foo_config in [
                "pkg-config --cflags --libs %s" % sdl_lib_name_pkgconfig,
                "%s --cflags --libs" % sdl_config_name
                ]:
                try:
                    env.ParseConfig(foo_config)
                except OSError:
                    pass
                else:
                    break
        else:
            if sdldir:
                env.AppendUnique(CPPPATH = [os.path.join(sdldir, sdl_include_dir)], LIBPATH = [os.path.join(sdldir, "lib")])
            env.AppendUnique(CCFLAGS = ["-D_GNU_SOURCE"])
            env.AppendUnique(LIBS = Split("mingw32 %s %s" % (sdlmain_name, sdl_lib_name)))
            env.AppendUnique(LINKFLAGS = ["-mwindows"])
    else:
        if require_version:
            context.Message("Checking for %s library version >= %d.%d.%d... " % (sdl_lib, major_version, minor_version, patchlevel))
        else:
            context.Message("Checking for %s library... " % sdl_lib)
        context.env.AppendUnique(LIBS = [sdl_lib])
    test_program = """
        #include <%s.h> 
        \n""" % sdl_header
    if require_version:
        test_program += "#if SDL_VERSIONNUM(%s, %s, %s) < SDL_VERSIONNUM(%d, %d, %d)\n#error Library is too old!\n#endif\n" % \
            (sdl_lib.upper() + "_MAJOR_VERSION", \
             sdl_lib.upper() + "_MINOR_VERSION", \
             sdl_lib.upper() + "_PATCHLEVEL", \
             major_version, minor_version, patchlevel)
    test_program += """
        int main(int argc, char** argv)
        {
            SDL_Init(0);
            SDL_Quit();
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
#include <stdlib.h>
#include <stdio.h>

#include <SDL.h>
#include <SDL_mixer.h>

int main(int argc, char ** argv)
{
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stdout, "Cannot initialize SDL Audio: %s\\n", SDL_GetError());
        return (EXIT_FAILURE);
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
        fprintf(stdout, "Cannot initialize SDL Mixer: %s\\n", Mix_GetError());
        return (EXIT_FAILURE);
    }

    if (Mix_Init(MIX_INIT_OGG) != MIX_INIT_OGG) {
        fprintf(stdout, "Cannot initialize OGG codec: %s\\n", Mix_GetError());
        Mix_CloseAudio();
        return (EXIT_FAILURE);
    }

    Mix_Music* music = Mix_LoadMUS("$TESTFILE");
    if (music == NULL) {
        fprintf(stdout, "Cannot load music file: %s\\n", Mix_GetError());
        Mix_CloseAudio();
        return (EXIT_FAILURE);
    }

    fprintf(stdout, "Success\\n");
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    return (EXIT_SUCCESS);
}
\n
'''
    nodepath = File("data/core/music/main_menu.ogg").rfile().abspath.replace("\\", "\\\\")
    test_program1 = context.env.Clone(TESTFILE = nodepath).subst(test_program)
    #context.env.AppendUnique(LIBS = "SDL_mixer")
    context.Message("Checking for Ogg Vorbis support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True
    context.env["ENV"]["SDL_AUDIODRIVER"] = "dummy"
    (result, output) = context.TryRun(test_program1, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        test_program2 = context.env.Clone(TESTFILE = "data/core/music/main_menu.ogg").subst(test_program)
        (result, output) = context.TryRun(test_program2, ".c")
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
    nodepath = File("images/buttons/button_normal/button_H22-pressed.png").rfile().abspath.replace("\\", "\\\\")
    test_program1 = context.env.Clone(TESTFILE = nodepath).subst(test_program)
    context.Message("Checking for PNG support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True
    (result, output) = context.TryRun(test_program1, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        test_program2 = context.env.Clone(TESTFILE = "images/buttons/button_normal/button_H22-pressed.png").subst(test_program)
        (result, output) = context.TryRun(test_program2, ".c")
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
    nodepath = File("data/core/images/maps/background.jpg").rfile().abspath.replace("\\", "\\\\")
    test_program1 = context.env.Clone(TESTFILE = nodepath).subst(test_program)
    context.Message("Checking for JPG support in SDL... ")
    if context.env["host"]:
        context.Result("n/a (cross-compile)")
        return True
    (result, output) = context.TryRun(test_program1, ".c")
    if result:
        context.Result("yes")
        return True
    else:
        test_program2 = context.env.Clone(TESTFILE = "data/core/images/maps/background.jpg").subst(test_program)
        (result, output) = context.TryRun(test_program2, ".c")
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
