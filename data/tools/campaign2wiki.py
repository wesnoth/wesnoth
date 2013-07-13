#!/usr/bin/python
#-*- coding: utf-8 -*-

"""
A script that autogenerates some information about campaigns for the
CampaignInformation wiki page. The script is a WIP.
"""

#TODO: Make it work with all campaigns (see blacklist) and investigate
#wmlparser2.py issues. More information. Output in wiki format. Cleanup.

from __future__ import with_statement   # For python < 2.6
import glob, os.path, sys
try:
    import argparse
except ImportError:
    print('Please install argparse by running "easy_install argparse"')
    sys.exit(1)

import wesnoth.wmlparser2 as wmlparser2


class Campaign:
    """
    A class for a specific campaign.
    """
    def __init__(self, parser):
        self.parser = parser
        self.name = self.parser.get_text_val("name")
        self.id = self.parser.get_text_val("id")
        self.description = self.parser.get_text_val("description")
        # Difficulty levels are separated by commas, so there are
        # count(',')+1 difficulty levels.
        self.levels = self.parser.get_text_val("difficulties").count(',') + 1
        self.credits_link = "http://wiki.wesnoth.org/Credits#" + self.id
        #self.scenarios = self.build_scenario_list()
        #self.total_scenarios = len(self.scenarios)
        # Calculate how many scenarios a player will usually go through
        #self.normal_scenarios = max([int(i.number[:2]) for i in self.scenarios ])
        self.total_scenarios = self.normal_scenarios = 0

    def build_scenario_list(self):
        """
        Should find a better way to do this
        """
        scenarios = glob.glob(os.path.join('data/campaigns', self.name[:5]+ '*', 'scenarios/*.cfg'))
        print scenarios
        return [Scenario(self.name, args.wesnoth) for i in scenarios]


class Scenario:
    """
    A class for a specific scenario
    """
    def __init__(self, name, wesnoth):
        """
        We need wesnoth executable link to parse files.
        """
        self.parser = wmlparser2.Parser(wesnoth, None, None, False)
        self.number = self.parser.parse_file(path)
        for scenario in self.parser.get_all(tag='scenario'):
            self.turns = scenario.get_text_val("turns")
            self.number = scenario.get_text_val("id", val=0)[:scenario.get_text_val("id", val=0).find('_')]


def wiki_output(campaign):
    """
    Takes a campaign instance and outputs information in wiki format
    """
    # Remove Espreon fancy but bug-inducing characters
    # FIXME: This is not elegant at all, find a better way to do it
    for i in (u"’", u"—", u'‘'):
        campaign.name = ''.join(campaign.name.split(i))
        campaign.description = ''.join(campaign.description.split(i))
    text = """== {0} ==
{1}
Total scenarios : {2}
Scenarios you will go through : {3}
Difficulty levels : {4}
* [{5} Credits]
""".format(campaign.name, campaign.description, campaign.total_scenarios, campaign.normal_scenarios, campaign.levels, campaign.credits_link)
    return text


if __name__ == "__main__":
    # Possible arguments
    arg_parser = argparse.ArgumentParser(description='campaign2wiki is a script\
which generates information about campaigns for the wiki.')
    arg_parser.add_argument('-d', '--data', default='data/',
        dest='data_dir', help="The location of wesnoth data directory")
    arg_parser.add_argument('-o', '--output', default='/tmp/CampaignWML',
        dest='output_path', help="The location of the output file.")
    arg_parser.add_argument('-w', '--wesnoth', default='./wesnoth',
        dest='wesnoth', help='The wesnoth executable location')
    args = arg_parser.parse_args()

    output = ''
    main = wmlparser2.Parser(args.wesnoth, None, None, False)
    main.parse_file('data/_main.cfg')
    for campaign in main.get_all(tag='campaign'):
        a = Campaign(campaign)
        output += wiki_output(a)

    with open(args.output_path, "w") as wiki_format:
        wiki_format.write(output)
