import os, glob

output = []
for o in glob.glob("src/*.o") + glob.glob("src/*/*.o"):
    output.append((o, os.popen("nm -C %s" % o).read()))

output2 = os.popen("nm -C src/wesnoth").read()
output2 += os.popen("nm -C src/wesnoth_editor").read()
output2 += os.popen("nm -C src/campaignd").read()
output2 += os.popen("nm -C src/wesnoth").read()
output2 += os.popen("nm -C src/exploder").read()

def extract(line):
    return line[11:]

symbols2 = [extract(x) for x in output2.splitlines() if " T " in x]

for o in output:
    symbols1 = [extract(x) for x in o[1].splitlines() if " T " in x]

    found = []
    for symbol in symbols1:
        if not symbol in symbols2:
            found += [symbol]

    if found:
        print "%s:" % o[0]
        print "\n".join(found)
        print
