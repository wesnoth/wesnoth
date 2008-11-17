#!/usr/bin/python

# This is *not* a python AI, it's just run as AI so it can get access to
# Python's runtime documentation. This documentation then simply is dumped to
# stdout in a format ready to be pasted to the wiki.

def myhelp(topic, topics):
    """Collect all the help topics into the given list."""
    doc = getattr(eval(topic), "__doc__")
    subtopics = []
    for subtopic in getattr(eval(topic), "__dict__", []):
        if subtopic.startswith("_"): continue 
        myhelp(topic + "." + subtopic, subtopics)
    tc = getattr(eval(topic), "__class__", None)
    tt = getattr(tc, "__name__", None)
    if topic != "wesnoth.error":
        topics.append((topic, tt, doc, subtopics))

def output(topics, level):
    """Output the given topics in wiki format, in a given heading level."""
    color = 0
    topics.sort()
    for topic, tt, doc, subtopics in topics:
        dot = topic.rfind(".")
        if level == 1:
            print "==", topic[dot + 1:], "module reference =="
            print "''This is an automatically generated reference, but feel " +\
                "free to edit it - changes will not be overwritten but " +\
                "instead reviewed and included in the next version.''"
            print doc or "..."
            if subtopics:
                funcs = []
                others = []
                for s in subtopics:
                    if s[1] == "builtin_function_or_method":
                        funcs.append(s)
                    else:
                        others.append(s)
                if funcs:
                    print "=== Functions ==="
                    print "{|"
                    output(funcs, 3)
                    print "|}"
                output(others, 2)
        elif level == 2:
            print "===", topic[dot + 1:], "==="
            print doc or "..."
            if subtopics: 
                print "{|"
                output(subtopics, 3)
                print "|}"
        elif level == 3:
            options = " valign='top'"
            if color: options += " bgcolor='#FBF5EA'"
            print "|-" + options
            color = not color
            if tt in ["method_descriptor", "builtin_function_or_method"]:
                suffix = ""
                prefix = ""

                if doc and doc.startswith("Parameters:"):
                    l = doc.find("\n")
                    if l == -1: l = len(doc) - 1
                    suffix = "(" + doc[11:l].strip() + ")"
                    doc = doc[l + 1:]
                else:
                    suffix = "()"

                if doc and doc.startswith("Returns:"):
                    l = doc.find("\n")
                    if l == -1: l = len(doc) - 1
                    prefix = doc[8:l].strip() + " = "
                    doc = doc[l + 1:]

                print "|'''%s()'''" % topic[dot + 1:]
                print "|<code>%s%s%s</code>\n\n" % (prefix, topic[dot + 1:], suffix) +\
                    (doc and doc.replace("\n", "\n\n") or "...")
            else:
                print "|'''%s'''\n|%s" % (topic[dot + 1:],
                    (doc and doc.replace("\n", "\n\n") or "..."))

if __name__ == "__main__":
    import os
    # If we are run as script, run wesnoth with the --python-api switch.
    os.system("src/wesnoth --python-api")
else:
    # If we are run as a python script, output the documentation to stdout.
    import ai as wesnoth
    topics = []
    myhelp("wesnoth", topics)
    output(topics, 1)
