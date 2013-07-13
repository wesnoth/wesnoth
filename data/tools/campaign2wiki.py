#!/usr/bin/python
#-*- coding:utf-8 -*-

"""
A script that autogenerates some information about campaigns for the
CampaignInformation wiki page. The script is a WIP.
"""

import glob, os.path, argparse
import wesnoth.wmlparser2 as wmlparser2

class Campaign:
    """
    A class for a specific campaign.
    """
    def __init__(self, path, wesnoth):
        """
        We need wesnoth executable link to parse files.
        """
        self.wesnoth = wesnoth
        self.parser = wmlparser2.Parser(wesnoth, None, None, True)
        self.parser.parse_file(os.path.join(path, '_main.cfg'))
        for campaign in self.parser.get_all(tag = 'campaign'):
            self.name = campaign.get_text_val("name")
            self.id = campaign.get_text_val("id")
        self.credits_link = "http://wiki.wesnoth.org/Credits#" + self.id
        self.scenarios = self.build_scenario_list(path)
        self.total_scenarios = len(self.scenarios)
        # Calculate how many scenarios a player will usually go through
        self.normal_scenarios = max([int(i.number[:2]) for i in self.scenarios ])

    def build_scenario_list(self, path):
        scenarios = glob.glob(os.path.join(path, 'scenarios/*.cfg'))
        return [Scenario(i, self.wesnoth) for i in scenarios]

class Scenario:
    """
    A class for a specific scenario
    """
    def __init__(self, path, wesnoth):
        """
        We need wesnoth executable link to parse files.
        """
        self.path = path
        self.parser = wmlparser2.Parser(wesnoth, None, None, True)
        self.number = self.parser.parse_file(path)
        for scenario in self.parser.get_all(tag = 'scenario'):
            self.turns = scenario.get_text_val("turns")
            self.number = scenario.get_text_val("id", val=0)[:scenario.get_text_val("id", val=0).find('_')]      

if __name__ == "__main__":
    # Possible arguments
    arg_parser = argparse.ArgumentParser(description='campaign2wiki is a script\
which generates information about campaigns for the wiki.')
    arg_parser.add_argument('-d', '--data', default='data/',
dest='data_dir', help="The location of wesnoth data directory")
    arg_parser.add_argument('-o', '--output', default='/tmp/CampaignWML',
dest='output-path', help="The location of the output file.")
    arg_parser.add_argument('-w', '--wesnoth', default='./wesnoth',
dest='wesnoth', help='The wesnoth executable location')
    args = arg_parser.parse_args()

    # TODO:Blacklist : investigate each one of these and see where the issues are
    blacklist = ['Delfadors_Memoirs', 'Eastern_Invasion', 'Legend_of_Wesmere',
'Liberty', 'Northern_Rebirth', 'Sceptre_of_Fire', 'Son_Of_The_Black_Eye',
'The_Rise_Of_Wesnoth', 'tutorial']
    blacklist = ['data/campaigns/' + i for i in blacklist]

    for campaign in glob.glob(os.path.join(args.data_dir, 'campaigns/*')):
        if campaign not in blacklist: a = Campaign(campaign, args.wesnoth)
        print 'Campaign name : %s, id : %s, number of scenarios : %s, number of played scenarios : %s' % (a.name, a.id, a.total_scenarios, a.normal_scenarios)
