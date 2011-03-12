#!/usr/bin/env python
#
# Automagically set the village_per_scout parameters in MP scenarios.

import sys
import os
import getopt
import re

overwrite = False
defaultValue = 4
suffix = ''

RE_SCENARIO = '.*?\[multiplayer\].*?\[\/multiplayer\]'
scenario_block = re.compile(RE_SCENARIO, re.DOTALL)

side_block = re.compile('( |\t)*\[side\].*?\[\/side\]( |\t)*\n', re.DOTALL)

ai_block = re.compile('( |\t)*\[ai\].*?\[\/ai\]( |\t)*\n', re.DOTALL)

AI_SIDE = re.compile('(?P<text>( |\t)*side=(\w| |\-|\,|\t)*)\n', re.DOTALL)
AI_TIME = re.compile('(?P<text>( |\t)*time_of_day=(\w| |\-|\,|\t)*)', re.DOTALL)
AI_TURNS = re.compile('(?P<text>( |\t)*turns=(\w| |\-|\,|\t)*)', re.DOTALL)
AI_SCOUTS = re.compile('(?P<text>( |\t)*villages_per_scout=(\w| |\-|\,|\t)*)', re.DOTALL)
AI_SCOUTS_VALUE = re.compile('(?P<text>(?<=villages_per_scout=)(\d)*)', re.DOTALL)
AI_CAN_RECRUIT = re.compile('(?P<text>(?<=canrecruit=)(\d)*)', re.DOTALL)
AI_START = re.compile('(?P<text>( |\t)*\[ai\](\w| |\t)*\n)', re.DOTALL)

IF_TEXT = "[if]\n"
ENDIF_TEXT = "[/if]\n"
ELSE_TEXT = "[else]\n"
ENDELSE_TEXT = "[/else]"

SCOUTS_TEXT = "\n\tvillages_per_scout=0"
AI_SCOUTS_TEXT = "\n\t[ai]%s\n\t[/ai]" % SCOUTS_TEXT.replace('\n','\n\t')

def applySearch(text, RE, groupId):
    data = RE.search(text, 0)
    if data != None:
        return data.group(groupId)
    else:
        return ""

def updateDescription(ai, sides):
    if ai.value != "":
        new = defaultValue * sides
        ai.updated_description = ai.updated_description.replace(ai.value, str(new))

def getIndent(itemText, subitemText):
    item = re.compile('^( |\t)*').search(itemText).group()
    subitem = re.compile('^( |\t)*').search(subitemText).group()
    if item == '' or subitem == '':
        return subitem
    if item[0] == '\t' and subitem[0] == '\t':
        return (len(subitem) - len(item)) * '\t'
    if item[0] == ' ' and subitem[0] == ' ':
        return (len(subitem) - len(item)) * ' '
    return '\t'

class wikiAi:
    def __init__(self):
        self.start = ""
        self.scouts = ""
        self.full_description = ""
        self.updated_description = ""

    def addAiData(self, aiContent):
        if aiContent != None:
            self.start = applySearch(aiContent, AI_START, 'text')
            self.scouts = applySearch(aiContent, AI_SCOUTS, 'text')
            self.full_description = aiContent
            self.updated_description = aiContent
            self.value = applySearch(aiContent, AI_SCOUTS_VALUE, 'text')

class wikiAiList(list):
    def __str__(self):
        output = ""
        for item in self:
            output = output + item.full_description + " ; "
        return output

class wikiSide:
    def __init__(self):
        self.full_description = ''
        self.updated_description = ''
        self.side = ''
        # Will only contain one element
        self.ai = wikiAiList()
        self.scouts_setting = False

    def addAiData(self, sideContent):
        if sideContent != None:
            aiDetail = ai_block.search(sideContent, 0)
            while aiDetail != None:
                if applySearch(aiDetail.group(), AI_TIME, 'text') == "" and applySearch(aiDetail.group(), AI_TURNS, 'text') == "":
                    self.ai.append(wikiAi())
                    self.ai[self.getCurrentAiNumber()].addAiData(aiDetail.group())
                    if self.ai[self.getCurrentAiNumber()].scouts != "":
                        self.scouts_setting = True
                    break
                searchStart = aiDetail.end()
                aiDetail = ai_block.search(sideContent, searchStart)


    def updateAi(self, sides):
        if not len(self.ai):
            self.ai.append(wikiAi())
            space = re.compile('^( |\t)*').search(self.full_description).group()
            indent = getIndent(self.full_description, self.side)
            side_scout_text = AI_SCOUTS_TEXT.replace('\t', indent)
            side_scout_text = side_scout_text.replace('\n', '\n' + space)
            self.ai[self.getCurrentAiNumber()].addAiData(side_scout_text)
            self.updated_description = self.updated_description.replace('\n', self.ai[self.getCurrentAiNumber()].full_description + '\n', 1)
            updateDescription(self.ai[0], sides)
        else:
            if not self.scouts_setting:
                space = re.compile('^( |\t)*').search(self.full_description).group()
                indent = getIndent(self.full_description, self.side)
                side_scout_text = AI_SCOUTS_TEXT.replace('\t', indent)
                side_scout_text = side_scout_text.replace('\n', '\n' + space)
                self.ai[0].updated_description = self.ai[0].updated_description.replace(self.ai[0].start, self.ai[0].start.replace('\n', side_scout_text + '\n'))
                updateDescription(self.ai[0], sides)
            else:
                if overwrite:
                    updateDescription(self.ai[0], sides)
        if self.ai[0].full_description != self.ai[0].updated_description:
            self.updated_description = self.updated_description.replace(self.ai[0].full_description, self.ai[0].updated_description, 1)

    def getCurrentAiNumber(self):
        return len(self.ai) - 1

class wikiSideList(list):
    def __str__(self):
        output = ""
        for item in self:
            output = output + item.full_description + " ; "
        return output

class wikiScenario:
    def __init__(self):
        self.side = wikiSideList()
        self.full_description = ''
        self.updated_description = ''

    def parseScenario (self, scenarioContent):
        self.addScenarioData(scenarioContent)
        sideDetail = side_block.search(scenarioContent, 0)
        while sideDetail != None:
            self.addSideData(sideDetail.group())
            self.addAiData(sideDetail.group())
            searchStart = sideDetail.end()
            sideDetail = side_block.search(scenarioContent, searchStart)
        self.updateAi()

    def addScenarioData(self, scenarioContent):
        self.full_description = scenarioContent
        self.updated_description = scenarioContent

    def addSideData(self, sideContent):
        canrecruit = applySearch(sideContent, AI_CAN_RECRUIT, 'text')
        if canrecruit == "0":
            return
        self.side.append(wikiSide())
        self.side[self.getCurrentSideNumber()].full_description = sideContent
        self.side[self.getCurrentSideNumber()].updated_description = sideContent
        self.side[self.getCurrentSideNumber()].side = applySearch(sideContent, AI_SIDE, 'text')

    def addAiData(self, aiContent):
        self.side[self.getCurrentSideNumber()].addAiData(aiContent)

    def updateAi(self):
        for side in self.side:
            side.updateAi(len(self.side))
        for side in self.side:
            if side.full_description != side.updated_description:
                self.updated_description = self.updated_description.replace(side.full_description, side.updated_description, 1)

    def getCurrentSideNumber(self):
        return len(self.side) - 1

class wikiScenarioList(list):
    def __str__(self):
        output = ""
        for scenario in self:
            output = output + scenario.full_description + " ; "
        return output

    def addScenario(self, scenario):
        self.append(scenario)

def parseAll(dirName, fileList):
    scenarioListIndex = 0
    scenarioList = wikiScenarioList()
    for fileName in fileList:
        if os.path.splitext(fileName)[1] != '.cfg':
            continue
        if os.path.isdir(os.path.join(dirName, fileName)):
            continue
        f = file(os.path.join(dirName, fileName))
        fileContent = f.read()
        f.close()
        searchStart = 0
        scenario = scenario_block.match(fileContent, searchStart)
        while scenario !=  None:
            scenarioList.addScenario(wikiScenario())
            scenarioList[scenarioListIndex].parseScenario(scenario.group(0))
            searchStart = scenario.end()
            scenario = scenario_block.search(fileContent, searchStart)
            scenarioListIndex += 1
        updated_file = fileContent
        for scenarioItem in scenarioList:
            if scenarioItem.full_description != scenarioItem.updated_description:
                updated_file = updated_file.replace(scenarioItem.full_description, scenarioItem.updated_description)
        if updated_file != fileContent:
            (basename_out, ext_out) = os.path.splitext(fileName)
            basename_out = basename_out + suffix + ext_out
            f = file(basename_out,'w')
            f.write(updated_file)
            f.close()

def printUsage():
    print """scoutDefault.py [-hRO] [-d directory] [-f file] [-x extension]
-h : print this message
-R : recursively parse directories
-O : overwrite village_per_scout value in scenario
-d : directory to look for file to parse
-f : name of the file to parse
-x : suffix to append to filename
Example of use:
  ./scoutDefault.py -h
    Get help
  ./scoutDefault.py -f 2p_Blitz.cfg -x _new
    Run the script and write output on 2p_Blitz_new.cfg
  ./scoutDefault.py -d /usr/local/share/wesnoth/data/scenarios
    Run the script on all file under that directory
  ./scoutDefault.py -R -d /usr/local/share/wesnoth
    Run the script on all directories under that directory
  ./scoutDefault.py -f 2p_Blitz.cfg -O
    Run the script on 2p_Blitz.cfg and delete previous value"""

recursive = False
entryPoint = os.getcwd()
entryFile = os.listdir(os.getcwd())
resourcesFile = {}
try:
    (opts, argsProper) = getopt.getopt(sys.argv[1:], 'ROhf:d:x:v:"')
except getopt.GetoptError, e:
    print 'Error parsing command-line arguments: %s' % e
    printUsage()
    sys.exit(1)
for (option, parameter) in opts:
    if option == '-h': # Print the commandline help and exit.
        printUsage()
        sys.exit(0)
    elif option == '-R':
        recursive = True
    elif option == '-O':
        overwrite = False
    elif option == '-d':
        if not os.path.exists(parameter):
            print 'Error: %s directory does not exist' % parameter
            sys.exit(1)
        elif not os.path.isdir(parameter):
            print 'Error: %s is not a directory' % parameter
            sys.exit(1)
        entryPoint = parameter
        entryFile = os.listdir(entryPoint)
    elif option == '-f':
        entryFile = []
        entryFile.append(os.path.basename(parameter))
        entryPoint = os.path.dirname(parameter)
    elif option == '-x':
        suffix = parameter

if recursive == True:
    os.path.walk(entryPoint, parseAll, resourcesFile)
else:
    parseAll(entryPoint, entryFile)
