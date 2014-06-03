# vi: syntax=python:et:ts=4
from glob import glob
from os.path import join

def backup_env(env, vars):
    backup = dict()
    for var in vars:
        backup[var] = env.get(var, [])
    return backup

def restore_env(env, backup):
    for var in backup.keys():
        env[var] = backup[var]

def find_include(prefixes, include_file, include_subdir, default_prefixes = True):
    if default_prefixes:
        prefixes = prefixes + ["/usr", "/usr/local", "/sw", "/sw/local", "/opt", "/opt/local"]
    all_includes = []
    for prefix in prefixes:
        path = join(prefix, "include", include_subdir, include_file)
        #print path
        for include in glob(path):
            all_includes.append((prefix, include))
    return all_includes
