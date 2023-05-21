# vi: syntax=python:et:ts=4
from os.path import join
import os
from SCons.Builder import Builder
from SCons.Util    import WhereIs
from config_check_utils import find_include

def exists():
    return True

def generate(env):
    env.AppendENVPath("PATH", join(env["gettextdir"], "bin"))
    env["MSGFMT"] = WhereIs("msgfmt")
    msgfmt = Builder(
        action = "$MSGFMT --check-domain --statistics -o $TARGET $SOURCE",
        src_suffix = ".po",
        suffix = ".mo",
        single_source = True
        )
    env["BUILDERS"]["Msgfmt"] = msgfmt

    env["MSGMERGE"] = WhereIs("msgmerge")
    msgmerge = Builder(
        action = "$MSGMERGE --backup=none --previous -U $TARGET $SOURCE",
        src_suffix = ".pot",
        suffix = ".po",
        single_source = True
        )
    env["BUILDERS"]["MsgMerge"] = msgmerge

    env["MSGINIT"] = WhereIs("msginit")
    msginit = Builder(
        action = "$MSGINIT -i $SOURCE -o $TARGET -l $MSGINIT_LINGUA --no-translator",
        src_suffix = ".pot",
        suffix = ".po",
        single_source = True
        )
    env["BUILDERS"]["MsgInit"] = msginit
    env["ENV"]["LANG"] = os.environ.get("LANG")
    env["MSGINIT_LINGUA"] = "C"

    def MsgInitMerge(env, target, source, **kw):
        if os.path.exists(target + ".po"):
            return env.MsgMerge(target, source, **kw)
        else:
            return env.MsgInit(target, source, **kw)
    env.AddMethod(MsgInitMerge)

    env["PO4A_UPDATEPO"] = WhereIs("po4a-updatepo")
    po4a_update_po = Builder(
        action = "$PO4A_UPDATEPO -f $PO4A_FORMAT ${''.join([' -m ' + str(source) for source in SOURCES])} -p $TARGET",
        )
    env["BUILDERS"]["Po4aUpdatePo"] = po4a_update_po

    env["PO4A_TRANSLATE"] = WhereIs("po4a-translate")
    po4a_translate = Builder(
        action = "$PO4A_TRANSLATE -f $PO4A_FORMAT -L $PO4A_CHARSET -m ${SOURCES[0]} -p ${SOURCES[1]} -l $TARGET"
        )
    env["BUILDERS"]["Po4aTranslate"] = po4a_translate

config_checks = {}
