# vi: syntax=python:et:ts=4
# Shamelessly stolen from FreeOrion's SConstruct
# http://freeorion.svn.sourceforge.net/viewvc/freeorion/trunk/FreeOrion/SConstruct?revision=2478&view=markup

import os

def exists():
    return True

def generate(env):
        env['CCACHE'] = env.WhereIs("ccache")
        env['CC'] = '$CCACHE %s' % env['CC']
        env['CXX'] = '$CCACHE %s' % env['CXX']
        for i in ['HOME',
                  'CCACHE_DIR',
                  'CCACHE_TEMPDIR',
                  'CCACHE_LOGFILE',
                  'CCACHE_PATH',
                  'CCACHE_CC',
                  'CCACHE_PREFIX',
                  'CCACHE_DISABLE',
                  'CCACHE_READONLY',
                  'CCACHE_CPP2',
                  'CCACHE_NOSTATS',
                  'CCACHE_NLEVELS',
                  'CCACHE_HARDLINK',
                  'CCACHE_RECACHE',
                  'CCACHE_UMASK',
                  'CCACHE_HASHDIR',
                  'CCACHE_UNIFY',
                  'CCACHE_EXTENSION']:
            if i in os.environ and i not in env:
                env['ENV'][i] = os.environ[i]

