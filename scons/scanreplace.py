# vi: syntax=python:et:ts=4
# Shamelessly stolen from scons wiki
# http://www.scons.org/wiki/ReplacementBuilder

try:
    from string import Template

    class PercentDelimitedTemplate(Template):
        delimiter = "%"
except ImportError:
    pass

def replace_action(target, source, env):
    open(str(target[0]), 'w').write(PercentDelimitedTemplate(open(str(source[0]), 'r').read()).substitute(env))
    env.Depends(target, env.Value(env.Dictionary()))

def replace_string(target, source, env):
    return "Generating '%s' from '%s'..." % (str(target[0]), str(source[0]))

def generate(env, **kw):
    action = env.Action(replace_action, replace_string)
    env['BUILDERS']['ScanReplace'] = env.Builder(action=action, src_suffix='.in', single_source=True)

def exists(env):
    return True
