# vi: syntax=python:et:ts=4
# Shamelessly stolen from FreeOrion's SConstruct
# http://freeorion.svn.sourceforge.net/viewvc/freeorion/trunk/FreeOrion/SConstruct?revision=2478&view=markup

import os

def exists():
    return True

def generate(env):
        print env["ENV"]["PATH"]
        env['DISTCC'] = env.WhereIs("distcc")
        env['CC'] = '$DISTCC %s' % env['CC']
        env['CXX'] = '$DISTCC %s' % env['CXX']
        for i in ['HOME',
                  'DISTCC_HOSTS',
                  'DISTCC_VERBOSE',
                  'DISTCC_LOG',
                  'DISTCC_FALLBACK',
                  'DISTCC_MMAP',
                  'DISTCC_SAVE_TEMPS',
                  'DISTCC_TCP_CORK',
                  'DISTCC_SSH'
                  ]:
            if os.environ.has_key(i) and not env.has_key(i):
                env['ENV'][i] = os.environ[i]
