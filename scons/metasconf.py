# vi: syntax=python:et:ts=4
from SCons.Script import *

def init_metasconf(env, modules):
    modules = map(__import__, modules)
    config_checks = {}
    for module in modules:
        config_checks.update(module.config_checks)
    return config_checks
