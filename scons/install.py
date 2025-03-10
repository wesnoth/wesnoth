# vi: syntax=python:et:ts=4
from SCons.Action import ActionFactory
import shutil
from SCons.Script import *
import os
import SCons.Node.FS
from subprocess import call, Popen, PIPE


def install_filtered_hook(target, source, env):
    copy_filter = env["copy_filter"]
    target = Flatten(target)
    if len(target) != len(source):
        raise ValueError("Number of targets doesn't match number of sources")

    def do_copy(target, source):
        if copy_filter(source):
            if os.path.isfile(source):
                if env["verbose"]:
                    print("cp %s %s" % (source, target))
                shutil.copy2(source, target)
            else:
                if not os.path.exists(target):
                    if env["verbose"]:
                        print("Make directory {}".format(target))
                    os.makedirs(target)
                for file in os.listdir(source):
                    do_copy(os.path.join(target, file), os.path.join(source, file))

    for target_dir, source_dir in zip(target, source):
        target_path = str(target_dir)
        source_path = str(source_dir)
        if not os.path.exists(target_path):
            os.makedirs(target_path)
        for d in (target_path, source_path):
            if not os.path.isdir(d):
                raise ValueError("%s is not a directory" % d)
        do_copy(target_path, source_path)


def hard_link(dest, src, symlink=False):
    try:
        if symlink:
            os.symlink(src, dest)
        else:
            os.link(src, dest)
    except OSError as e:
        if e.errno == 18:
            hard_link(dest, src, True)
        else:
            os.remove(dest)
            os.link(src, dest)
    except AttributeError:
        shutil.copy2(src, dest)


HardLink = ActionFactory(hard_link, lambda dest, src: 'Hardlinking %s to %s' % (src, dest))


def InstallBinary(env, source):
    if not source:
        return source

    binary = source[0].name
    binary = binary.split("-")[0]
    install_dir = env.subst(os.path.join(env["destdir"], env["bindir"].lstrip("/")))
    env.Alias("install-" + binary, env.InstallAs(os.path.join(install_dir, binary + env["program_suffix"]), source))


def InstallData(env, datadir, component, source, subdir="", **kwargs):
    install_dir = Dir(env.subst(os.path.join(env["destdir"], env[datadir].lstrip("/"), subdir)))
    sources = map(Entry, Flatten([source]))
    dirs = []
    for source in sources:
        if isinstance(source, SCons.Node.FS.Dir) or source.isdir():
            dirs.append(source)
        else:
            env.Alias(
                "install-" + component, env.Install(install_dir, source, **kwargs)
            )
    if dirs:
        if len(dirs) == 1:
            install = env.InstallFiltered(install_dir.path, dirs[0].path, **kwargs)
        else:
            install = [env.InstallFiltered(os.path.join(install_dir.path, x.name, **kwargs), x.path) for x in dirs]
        AlwaysBuild(install)
        env.Alias("install-" + component, install)


def generate(env):
    env.AddMethod(InstallBinary)
    env.AddMethod(InstallData)

    env.Append(BUILDERS={"InstallFiltered": Builder(action=install_filtered_hook, target_factory=Dir, source_factory=Dir)})


def exists():
    return True
