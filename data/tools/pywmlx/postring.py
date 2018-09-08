import re



# PoCommentedStringPL represents a 'plural' form of a PoCommentedString or a
# WmlNode. Currently PoCommentedStringPL is actually used only within
# PoCommentedString
# (until wesnoth supports plural forms only on lua code)
class PoCommentedStringPL:
    def __init__(self, value, *, ismultiline=False):
        self.value = value
        self.ismultiline = ismultiline



class PoCommentedString:
    def __init__(self, sentence, *, orderid, ismultiline,
                 wmlinfos, finfos, addedinfos, plural=None):
        self.sentence = sentence
        self.wmlinfos = wmlinfos
        self.addedinfos = addedinfos
        self.finfos = finfos
        self.orderid = orderid
        self.ismultiline = ismultiline
        self.plural = None
        if isinstance(plural, PoCommentedStringPL):
            self.plural = plural

    def set_plural(self, plural_value, *, ismultiline=False):
        # set_plural is a safe way to add a plural form into a sentence
        # if plural form is still stored, no plural form is added
        if self.plural is None:
            self.plural = PoCommentedStringPL(plural_value, ismultiline)

    def update_orderid(self, orderid):
        if orderid < self.orderid:
            self.orderid = orderid

    def update_with_commented_string(self, commented_string):
        if commented_string.wmlinfos:
            if commented_string.wmlinfos[0] not in self.wmlinfos:
                self.wmlinfos.append(commented_string.wmlinfos[0])
        if commented_string.addedinfos:
            self.addedinfos += commented_string.addedinfos
        self.finfos += commented_string.finfos
        self.update_orderid(commented_string.orderid)
        # update plural value only if current sentence actually don't have
        # a plural form
        if self.plural is None and isinstance(commented_string.plural,
                                              PoCommentedStringPL):
            self.plural = commented_string.plural

    def write(self, filebuf, fuzzy):
        if self.wmlinfos is not None:
            for i in self.wmlinfos:
                print('#.', i, file=filebuf)
        if self.addedinfos is not None:
            for i in self.addedinfos:
                print('#.', i, file=filebuf)
        if self.finfos is not None:
            for i in self.finfos:
                print('#:', i, file=filebuf)
        if fuzzy:
            print('#, fuzzy', file=filebuf)
        self.sentence = '"' + self.sentence + '"'
        if self.ismultiline:
            lf = r'\\n"' + '\n"'
            self.sentence = re.sub(r'(\n\r|\r\n|[\n\r])', lf, self.sentence)
            self.sentence = '""\n' + self.sentence
        print('msgid', self.sentence, file=filebuf)
        if self.plural is None:
            print('msgstr ""', file=filebuf)
        else:
            # self.plural must be an instance of PoCommentedStringPL
            print('msgid_plural', self.plural.value, file=filebuf)
            print('msgstr[0] ""', file=filebuf)
            print('msgstr[1] ""', file=filebuf)


# WmlNodeSentence use PoCommentedStringPL for 'plural' parameter
class WmlNodeSentence:
    def __init__(self, sentence, *, ismultiline, lineno, lineno_sub=0,
                 plural=None, override=None, addition=None):
        self.sentence = sentence
        # Say if it is multiline or not.
        self.ismultiline = ismultiline
        self.lineno = lineno
        # lineno_sub:
        # used only in WmlNodeSentence. This parameter is actually used
        # only when multiple translatable strings were found on the same line.
        # On PoCommentedString translation a lineno_sub != 0
        # will translated as a floated lineno value
        #      (ex. lineno = 10.02 means "lineno = 10, lineno_sub = 2)
        self.lineno_sub = lineno_sub
        # overrideinfo is usually None.
        # overrideinfo will be used when an override wml info
        # was found into lua or Wml code ->
        #                   (overrideinfo will be a non-empty string)
        # every translated string can have only ONE override info.
        self.overrideinfo = override
        # addedinfo is usually an empty list.
        # list will contains custom comment that will be added at the
        # end of translator's comment list.
        self.addedinfo = addition
        if addition is None:
            self.addedinfo = []
        # plural forms (if any) will be stored in WmlNodeSentence
        self.plural = None
        if isinstance(plural, PoCommentedStringPL):
            self.plural = plural

    def set_plural(self, plural_value, *, ismultiline=False):
        # add_plural is a safe way to add a plural form into a sentence
        # if plural form is still stored, no plural form is added
        if self.plural is None:
            self.plural = PoCommentedStringPL(plural_value, ismultiline)



class WmlNode:
    def __init__(self, fileref, fileno, tagname, *, autowml=True):
        self.tagname = tagname
        self.fileref = fileref + ":"
        self.fileno = fileno
        self.sentences = None
        self.wmlinfos = None
        self.autowml = autowml

    def add_sentence(self, sentence, *, ismultiline, lineno,
                     lineno_sub=0, plural=None, override=None, addition=None):
        if self.sentences is None:
            self.sentences = []
        # 'plural' parameter accepted by WmlNode.add_sentence can be:
        #     1) A string
        #     2) A tuple of two values (string, bool)
        #     3) An actual PoCommentedStringPL object
        # This is why we need to add an extra variable here (plural_value)
        # wich will store an PoCommentedStringPL object (or will be None)
        plural_value = None
        if plural is not None:
            if isinstance(plural, str):
                plural_value = PoCommentedStringPL(plural)
            elif isinstance(plural, tuple) or isinstance(plural, list):
                if len(plural) == 2:
                    if(isinstance(plural[0], str) and
                          isinstance(plural[1], bool)):
                        plural_value = PoCommentedStringPL(plural[0],
                                                           plural[1])
                    elif(isinstance(plural[0], bool) and
                          isinstance(plural[1], str)):
                        plural_value = PoCommentedStringPL(plural[1],
                                                           plural[0])
            elif isinstance(plural, PoCommentedStringPL):
                plural_value = plural
                plural = None
        self.sentences.append( WmlNodeSentence(sentence,
                                          ismultiline=ismultiline,
                                          lineno=lineno,
                                          lineno_sub=lineno_sub,
                                          plural=plural_value,
                                          override=override,
                                          addition=addition) )

    def assemble_wmlinfo(self):
        if self.tagname == "":
            value = []
            return value
        else:
            mystr = self.tagname
            intstr = 0
            if self.wmlinfos is not None:
                if len(self.wmlinfos) > 0:
                    mystr += ": "
                    for k in self.wmlinfos:
                        intstr += 1
                        if intstr > 1:
                            mystr += ", "
                        mystr += k
            return mystr

    def assemble_orderid(self, nodesentence):
        return (self.fileno, nodesentence.lineno,
                nodesentence.lineno_sub)

    def nodesentence_to_posentence(self, nodesentence):
        # if nodesentence has overrideinfo, overrideinfo will be written
        # instead of the automatic wml infos.
        # if it has an EMPTY overrideinfo, no wml infos will be added
        #             (this happens if a macro call encountered)
        # also no wml infos will be added if "autowml" is false
        #             (this happens: on root node, on [lua] tags.
        #              on #define nodes)
        # but if "autowml" is false AND an overrideinfo is given, than
        # overridden wmlinfo is added.
        # added_infos are also infos for translators, but it will be written
        # after wml_infos, in addition to them
        if nodesentence.overrideinfo is not None:
            if nodesentence.overrideinfo == "":
                if(nodesentence.addedinfo is not None and
                              nodesentence.addedinfo != ""):
                    return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wmlinfos=[],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=nodesentence.addedinfo,
                               plural=nodesentence.plural )
                else:
                    return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wmlinfos=[],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=[],
                               plural=nodesentence.plural )
            else: # having a non-empty override
                if(nodesentence.addedinfo is not None and
                              nodesentence.addedinfo != ""):
                    return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wmlinfos=[nodesentence.overrideinfo],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=nodesentence.addedinfo,
                               plural=nodesentence.plural )
                else:
                    return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wmlinfos=[nodesentence.overrideinfo],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=[],
                               plural=nodesentence.plural )
        # if you don't have override and autowml is true
        #  --> wmlinfos will be always added
        elif self.autowml == True:
            if(nodesentence.addedinfo is not None and
                          nodesentence.addedinfo != ""):
                return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wmlinfos=[self.assemble_wmlinfo()],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=nodesentence.addedinfo,
                               plural=nodesentence.plural )
            else:
                return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wmlinfos=[self.assemble_wmlinfo()],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=[],
                               plural=nodesentence.plural )
        # if you don't have override and autowml is false
        #  --> wmlinfos will never added
        else:
            if(nodesentence.addedinfo is not None and
                          nodesentence.addedinfo != ""):
                return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wmlinfos=[],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=nodesentence.addedinfo,
                               plural=nodesentence.plural )
            else:
                return PoCommentedString(nodesentence.sentence,
                               ismultiline=nodesentence.ismultiline,
                               orderid=self.assemble_orderid(nodesentence),
                               wml_infos=[],
                               finfos=[self.fileref +
                                        str(nodesentence.lineno)],
                               addedinfos=[],
                               plural=nodesentence.plural )
