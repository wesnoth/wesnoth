# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:

"""
This library provides an interface to github, the interface is build upon
the command line git tool.
"""

import logging
import os
try:
    # Externally distributed, usually more up-to-date
    import simplejson as json
except:
    # Distributed with python since 2.6
    import json
import subprocess
import urllib2


#TODO: document and log where missing

class Error(Exception):
    """Base class for exceptions in this module."""
    pass

class Addon(object):
    """Represents an add-on from a github directory.

    Each Addon object belongs to GitHub object and should not be created manually.
    """
    def __init__(self, github, name):
        """Initialize an Addon object.

        Do NOT use this constructor directly.

        github: Parent GitHub object that created this object.
        name: Name of the add-on that this object represents.
        """
        logging.debug("Addon created with name {0} and version {1}".format(name, github.version))
        self.github = github
        self.name = name

    def erase(self):
        """Erase this add-on.

        Always raises NotImplementedError.
        """
        logging.debug("Erasing add-on {0}".format(self.name))
        raise NotImplementedError("This would cause the permanent loss of the repository, including history.")

    def update(self):
        """Update this add-on.

        Returns whether anything changed.
        """
        logging.debug("Updating add-on {0}".format(self.name))
        out, err = self._execute(["git", "pull"])
        #TODO: make this less hacky
        return len(out.split("\n")) > 2

    #TODO: any commit has to be ended with "git push -u origin master"

    def _get_dir(self):
        return os.path.join(self.github.directory, self.name)
    def _execute(self, command):
        return self.github._execute(command, cwd=self._get_dir())

_GITHUB_API_BASE = "https://api.github.com/"
_GITHUB_API_REPOS = "orgs/wescamp/repos"

class GitHub(object):
    """Interface to a github checkout directory. Such a directory contains all translatable add-ons for a certain wesnoth version.

    Every GitHub object is specific to a directory and wesnoth version.
    """
    def __init__(self, directory, version):
        """Initializes a GitHub object.

        Directory: Directory in which the git repos for this wesnoth branch live.
        Version: The version of this wesnoth branch.
        """
        logging.debug("GitHub created with directory {0} and version {1}".format(directory, version))
        self.directory = directory
        self.version = version


    def update(self):
        """Update all add-ons.

        Returns whether anything changed.
        """
        logging.debug("Updating in directory {0}".format(self.directory))
        changed = False
        changed |= self._get_new_addons()

        for addon in self._get_local_addons():
            changed |= addon.update()

        return changed

    def addon(self, name):
        """Returns an add-on object for the given name.

        Raises libgithub.Error if no such add-on exists.
        """
        logging.debug("Generating add-on object for {0}".format(name))
        if not os.path.isdir(self._absolute_path(name)):
            logging.debug("Add-on {0} not found locally, checking github.".format(name))
            github_list = self._github_repos_list()
            if name in [repo[0] for repo in github_list]:
                self._clone(repo[0], repo[1])
            else:
                raise Error("No such add-on found")
        return Addon(self, name)

    def create_addon(self, name):
        """Creates a new add-on on github.

        name: Name of the add-on
        Returns an Addon object for the new add-on.
        """
        logging.debug("Creating new add-on {0}".format(name))
        response = self._github_repos_create(name)
        parsed = json.load(response)
        self._clone(name, parsed["ssh_url"])
        return self.addon(name)

    def _absolute_path(self, name):
        return os.path.join(self.directory, name)

    def _clone(self, name, url):
        target = self._absolute_path(name)
        self._execute(["git", "clone", url, target])

    def _get_new_addons(self):
        """Checks out any new add-ons

        Returns whether anything changed.
        """
        changed = False
        github_list = self._github_repos_list()
        local_list = self._get_local_addons
        for repo in github_list:
            if repo[0] not in local_list:
                self._clone(repo[0], repo[1])
                changed = True
        return changed

    def _get_local_addons(self):
        """...

        Returns list of local add-ons.
        """
        return os.listdir(self.directory)

    def _github_repos_list(self):
        """Get a list of repositories.

        Returns a list of tuples that contain the add-on name and the url.
        """
        url = _GITHUB_API_BASE + _GITHUB_API_REPOS
        try:
            response = urllib2.urlopen(url)
        except IOError as e:
            raise Error("GitHub API failure: " + str(e))
        repos = json.load(response)

        version_suffix = "-{0}".format(self.version)
        return [(repo["name"][:-len(version_suffix)], repo["git_url"])
                for repo in repos if repo["name"].endswith(version_suffix)]

    def _github_repos_create(self, name):
        """Create a new repository

        name: The name of the add-on for which the repository will be created.
        """
        url = _GITHUB_API_BASE + _GITHUB_API_REPOS
        reponame = "{0}-{1}".format(name, self.version)
        request = { "name" : reponame }
        request = json.dumps(request)
        #TODO: authentication, either OAuth or basic authentication
        # Basic works in curl, but urllib2 doesn't
        # probably because github's API doesn't send a www-authenticate header
        #also TODO: adding to correct teams
        # PUT /teams/:id/repos/:user/:repo
        try:
            urllib2.urlopen(url, request)
        except IOError as e:
            raise Error("GitHub API failure: " + str(e))

    def _execute(self, command, cwd=None):
        """Executes a command.

        command: The command to execute
        Returns stdout, stderr
        """
        logging.debug("execute command = '%s'", command)

        p = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE, close_fds=True, cwd=cwd)
        out = ""
        err = ""
        while(p.poll() == None):
            out += p.stdout.read()
            err += p.stderr.read()

        out += p.stdout.read()
        err += p.stderr.read()

        logging.debug("===== stdout ====\n\n\n%s\n\n\n===== stdout ====", out)
        logging.debug("===== stderr ====\n\n\n%s\n\n\n===== stderr ====", err)

        return out, err

