# vi: syntax=python:et:ts=4

def backup_env(env, vars):
    backup = dict()
    for var in vars:
        backup[var] = env.get(var, [])
    return backup

def restore_env(env, backup):
    for var in backup.keys():
        env[var] = backup[var]
