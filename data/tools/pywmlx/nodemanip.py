import re

import pywmlx.postring as pos
from pywmlx.wmlerr import wmlerr
from pywmlx.wmlerr import wmlwarn


fileref = None
fileno = None
nodes = None
onDefineMacro = False



def _closenode_update_dict(podict):
    if nodes[-1].sentences is not None:
        for i in nodes[-1].sentences:
            posentence = podict.get(i.sentence)
            if posentence is None:
                podict[i.sentence] = (
                       nodes[-1].nodesentence_to_posentence(i) )
            else:
                posentence.update_with_commented_string(
                       nodes[-1].nodesentence_to_posentence(i) )



def newfile(file_ref, file_no):
    global fileref
    global fileno
    global nodes
    global onDefineMacro
    onDefineMacro = False
    fileref = file_ref
    fileno = file_no
    if nodes is not None:
        del nodes[:]
    nodes = None



def closefile(mydict, lineno):
    if nodes is not None:
        if len(nodes) > 1:
            err_message = ("End of WML file reached, but some tags were " +
                            "not properly closed.\n"
                            "(nearest unclosed tag is: " +
                            nodes[-1].tagname + ")" )
            finfo = fileref + ":" + str(lineno)
            wmlerr(finfo, err_message)
        else:
            _closenode_update_dict(mydict)


def newnode(tagname):
    global fileref
    global fileno
    global nodes
    if nodes is None:
        nodes = [pos.WmlNode(fileref, fileno, "", autowml=False)]
    if tagname == "[lua]":
        nodes.append( pos.WmlNode(fileref, fileno,
                                  tagname, autowml=False) )
    else:
        nodes.append( pos.WmlNode(fileref, fileno,
                                  tagname, autowml=True) )



def closenode(closetag, mydict, lineno):
    global fileref
    global fileno
    global nodes
    if nodes is None:
        err_message = ("unexpected closing tag '" +
                            closetag + "' outside any scope.")
        finfo = fileref + ":" + str(lineno)
        wmlerr(finfo, err_message)
    else:
        # node to close is the LAST element in self.nodes list
        mytag = nodes[-1].tagname
        mynode = nodes[-1]
        expected_closetag = re.sub(r'\[', r'[/', mytag)
        finfo = fileref + ":" + str(lineno)
        if mynode.tagname == "":
            err_message = ("unexpected closing tag '" +
                            closetag + "' outside any scope.")
            wmlerr(finfo, err_message)
        else:
            if closetag != expected_closetag:
                err_message = ("expected closing tag '" +
                                expected_closetag + "' but '" +
                                closetag + "' found.")
                wmlerr(finfo, err_message)
        _closenode_update_dict(mydict)
        nodes.pop()


def addNodeSentence(sentence, *, ismultiline, lineno, lineno_sub,
                    override, addition, plural=None):
    global nodes
    if nodes is None:
        nodes = [pos.WmlNode(fileref=fileref, fileno=fileno,
                              tagname="", autowml=False)]
    nodes[-1].add_sentence(sentence, ismultiline=ismultiline,
                           lineno=lineno, lineno_sub=lineno_sub,
                           override=override, addition=addition,
                           plural=plural)


def addWmlInfo(info):
    global nodes
    if nodes is None:
        nodes = [pos.WmlNode(fileref=fileref, fileno=fileno,
                              tagname="", autowml=False)]
    if nodes[-1].wmlinfos is None:
        nodes[-1].wmlinfos = []
    nodes[-1].wmlinfos.append(info)
