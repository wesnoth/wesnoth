#!/usr/bin/env python3
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
except ImportError:
    # Distributed with python since 2.6
    import json
import shutil
import subprocess
import tempfile
import urllib.request


#TODO: document and log where missing

class Error(Exception):
    """Base class for exceptions in this module."""
    pass

class AddonError(Error):
    """Class for exceptions that belong to an add-on."""
    def __init__(self, addon, message):
        self.addon = addon
        self.message = message
        self.args = (addon, message)
    def __str__(self):
        return "{0}: {1}".format(str(self.addon), str(self.message))

class _execresult(object):
    """Store the results of GitHub._execute and Addon._execute"""
    def __init__(self, out, err, returncode):
        self.out = out
        self.err = err
        self.returncode = returncode
    def __iter__(self):
        yield self.out
        yield self.err
        yield self.returncode

class Addon(object):
    """Represents an add-on from a github directory.

    Each Addon object belongs to GitHub object and should not be created manually.
    """
    def __init__(self, github, name, readonly):
        """Initialize an Addon object.

        Do NOT use this constructor directly.

        github: Parent GitHub object that created this object.
        name: Name of the add-on that this object represents.
        readonly: Whether the add-on has been checked out over git: instead of ssh:
        """
        logging.debug("Addon created with name {0} and version {1}{2}".format(name, github.version, ". It is read-only" if readonly else ""))
        self.github = github
        self.name = name
        self.readonly = readonly

    def update(self):
        """Update this add-on.

        Returns whether anything changed.
        """
        logging.debug("Updating add-on {0}".format(self.name))
        out, err, ret = self._execute(["git", "pull"], check_error=False)
        if len(err):
            real_errs = []
            for line in err.splitlines():
                if line in ["Your configuration specifies to merge with the ref 'master'", "from the remote, but no such ref was fetched."]:
                    # This means the repository has no commits yet
                    pass
                elif "From" in line or "origin/master" in line:
                    # Regular fetch stuff
                    pass
                elif "Checking out files" in line:
                    # Irregular fetch stuff
                    # not being attached to a terminal *should* squelch progress reports
                    pass
                else:
                    real_errs.append(line)
            if real_errs:
                raise AddonError(self.name, "Error pulling:\n{0}".format("\n".join(real_errs)))


        def remove_untracked():
            untracked = [line.replace("?? ", "", 1) for line in self._status() if line.startswith("??")]
            for item in untracked:
                try:
                    path = os.path.join(self.get_dir(), item)
                    if item.endswith("/"):
                        shutil.rmtree(path)
                    else:
                        os.remove(path)
                except:
                    logging.error("Failed to remove {0}".format(item))

        if "Already up-to-date." in out:
            return False
        elif "Fast-forward" in out:
            return True
        elif "Merge made by recursive." in out:
            logging.warn("Merge done in add-on {0}.".format(self.name))
            return True
        elif "CONFLICT" in out:
            #This means that a conflicting local commit was done
            #Its author will have to fix it
            logging.error("CONFLICT in add-on {0}. Please merge".format(self.name))
            return False
        elif "local changes" in err:
            logging.error("Found local changes in add-on {0}.".format(self.name))
            # If this is a read-write repo, leave the files be
            # If it's read-only, they're not supposed to be here
            if self.readonly:
                logging.warn("Attempting to fix.")
                # Get rid of local modifications
                self._execute(["git", "reset", "--hard"], check_error=False)

                status = self._status()
                untracked = [line for line in status if "??" in line]
                # I don't want to recursively delete directories
                if len(untracked) > 0:
                    logging.warn("Untracked files found. Attempting to remove...")
                    remove_untracked()

            return False
        elif "Untracked working tree" in err:
            if self.readonly:
                logging.error("Untracked files blocking pull of {0}. Attempting to remove...".format(self.name))
                remove_untracked()
            else:
                logging.error("Untracked files blocking pull of {0}. Please remove.".format(self.name))
            return False
        elif "Your configuration specifies to merge with the ref 'master'" in err:
            logging.info("Pulled from still-empty (not initialized) repository {0}.".format(self.name))
            return False
        else:
            logging.error("Unknown pull result in add-on {0}:\nOut: {1}\nErr: {2}".format(self.name, out, err))
            return False

    def sync_from(self, src, exclude):
        """Synchronises add-on from another directory.

        src: Directory with new add-on version.
        exclude: List of files to ignore.
        Returns whether anything changed.
        Raises libgithub.Error if the checkout is not clean.
        """
        logging.debug("Syncing add-on {0} from add-on server ({1})".format(self.name, src))

        status = self._status()
        if status:
            raise AddonError(self.name, "Checkout is not clean:\n{0}".format("\n".join(status)))

        self._rmtree(".", exclude)
        #actual copying
        self._copytree(src, self.get_dir(), ignore=lambda src, names: [n for n in names if n in exclude])
        self._execute(["git", "add", "."], check_error=True)

        status = self._status()
        return len(status) > 0

    def commit(self, message):
        """Commits and pushes add-on to git repo.

        message: Commit message.
        Raises libgithub.Error if something went wrong
        """
        logging.debug("Committing and pushing add-on {0}".format(self.name))

        tmpfile = tempfile.NamedTemporaryFile(delete=False)
        tmpfile.write(message)
        tmpfile.close()
        tmpname = tmpfile.name
        self._execute(["git", "commit", "-F", tmpname], check_error=True)
        os.remove(tmpname)
        out, err, ret = self._execute(["git", "push", "-u", "--porcelain", "origin", "master"], check_error=False)
        statusline = [x for x in out.splitlines() if "refs/heads/master" in x]
        if not statusline:
            raise AddonError(self.name, "No statusline produced by git push")
        else:
            status = statusline[0][0]
            refs, summary = statusline[0][1:].split(None, 1)
            if status == " ":
                # Fast forward
                pass
            elif status == "*":
                # Freshly initiated repository
                pass
            elif status == "=":
                # Up to date?
                logging.warn("Commit to add-on {0} with message '{1}' has not made any changes".format(self.name, message))
            elif status == "!":
                raise AddonError(self.name, "Commit with message '{0}' failed for reason {1}".format(message, summary))
            else:
                raise AddonError(self.name, "Commit with message '{0}' has done something unexpected: {1}".format(message, statusline[0]))

    def get_dir(self):
        """Return the directory this add-on's checkout is in.
        """
        return os.path.join(self.github.directory, self.name)

    # Internal functions

    def _rmtree(self, directory, exclude):
        logging.debug("Deleting tree {0}, except for {1}".format(self.name, ",".join(exclude)))

        # Ensure the os calls all happen in the right directory
        # not needed for _execute, as that does the cwd manipulation itself
        # so only the os.chdir and os.path.isdir here need it
        # Another option would be to os.path.join with self.get_dir
        os.chdir(self.get_dir())
        for entry in os.listdir(directory):
            if entry in exclude:
                continue
            if entry == ".git":
                continue

            relpath = os.path.join(directory, entry)

            if os.path.isdir(relpath):
                self._rmtree(relpath, exclude)
                # git rm removes directories that it empties
                if os.path.exists(relpath):
                    self._execute(["rmdir", "--ignore-fail-on-non-empty", relpath])
            else:
                self._execute(["git", "rm", relpath], check_error=True)

    def _copytree(self, src, dst, ignore=None):
        """Recursively copy a directory tree using copy2().

        Based on shutil.copytree
        """
        names = os.listdir(src)
        if ignore is not None:
            ignored_names = ignore(src, names)
        else:
            ignored_names = set()

        if not os.path.exists(dst):
            os.makedirs(dst)
        errors = []
        for name in names:
            if name in ignored_names:
                continue
            srcname = os.path.join(src, name)
            dstname = os.path.join(dst, name)
            try:
                if os.path.isdir(srcname):
                    self._copytree(srcname, dstname, ignore)
                else:
                    shutil.copy2(srcname, dstname)
                # XXX What about devices, sockets etc.?
            except (IOError, os.error) as why:
                errors.append((srcname, dstname, str(why)))
            # catch the Error from the recursive copytree so that we can
            # continue with other files
            except Error as err:
                errors.extend(err.args[0])
        try:
            shutil.copystat(src, dst)
        except OSError as why:
            if shutil.WindowsError is not None and isinstance(why, shutil.WindowsError):
                # Copying file access times may fail on Windows
                pass
            else:
                errors.extend((src, dst, str(why)))
        if errors:
            raise AddonError(self.name, "Errors attempting to sync:\n{0}".format("\n".join(errors)))
    def _status(self):
        out, err, ret = self._execute(["git", "status", "--porcelain"])
        if err:
            raise AddonError(self.name, "Status failed with message: {0}".format(err))
        return [line for line in out.split('\n') if len(line)]
    def _execute(self, command, check_error = False):
        return self.github._execute(command, cwd=self.get_dir(), check_error=check_error)

_GITHUB_API_BASE = "https://api.github.com/"
_GITHUB_API_REPOS = "orgs/wescamp/repos"
_GITHUB_API_TEAMS = "orgs/wescamp/teams"
# PUT /teams/:id/repos/:org/:repo
_GITHUB_API_TEAM_REPO = "teams/{0}/repos/wescamp/{1}"
# POST /repos/:user/:repo/hooks
_GITHUB_API_HOOKS = "repos/wescamp/{0}/hooks"

class GitHub(object):
    """Interface to a github checkout directory. Such a directory contains all translatable add-ons for a certain wesnoth version.

    Every GitHub object is specific to a directory and wesnoth version.
    """
    def __init__(self, directory, version, authorization=None):
        """Initializes a GitHub object.

        directory: Directory in which the git repos for this wesnoth branch live.
        version: The version of this wesnoth branch.
        """
        logging.debug("GitHub created with directory {0} and version {1}, {2} authentication data".format(directory, version, "with" if authorization else "without"))
        self.directory = directory
        self.version = version
        self.authorization = authorization

    def update(self):
        """Update all add-ons.

        Returns whether anything changed.
        """
        logging.debug("Updating in directory {0}".format(self.directory))
        changed = False
        changed |= self._get_new_addons()

        for addon in self._get_local_addons():
            changed |= self.addon(addon).update()

        return changed

    def addon(self, name, readonly=False):
        """Returns an add-on object for the given name.

        name: Name of the add-on.
        readonly: If set, and the add-on needs to be freshly cloned, use a read-only protocol

        Raises libgithub.Error if no such add-on exists.
        """
        logging.debug("Generating add-on object for {0}".format(name))
        if not os.path.isdir(self._absolute_path(name)):
            logging.debug("Add-on {0} not found locally, checking github.".format(name))
            github_list = self._github_repos_list(readonly=readonly)
            matches = [x for x in github_list if x[0] == name]
            if matches:
                repo = matches[0]
                self._clone(repo[0], repo[1])
            else:
                raise AddonError(name, "Add-on not found")
        return Addon(self, name, readonly)

    def create_addon(self, name):
        """Creates a new add-on on github.

        name: Name of the add-on.
        Returns an Addon object for the new add-on.
        """
        logging.debug("Creating new add-on {0}".format(name))
        response = self._github_repos_create(name)
        self._clone(name, response["ssh_url"])
        return self.addon(name)

    def addon_exists(self, name):
        """Checks whether an add-on exists on github..

        name: Name of the add-on.
        Returns a bool representing the existence of the add-on.
        """
        logging.debug("Checking whether add-on {0} exists".format(name))
        github_list = self._github_repos_list()
        return name in [repo[0] for repo in github_list]

    def list_addons(self):
        """Returns a list of valid add-on names.

        Returns a list of names that can be passed to self.addon()
        """
        logging.debug("Generating list of add-on names for version {0}".format(self.version))
        github_list = self._github_repos_list()
        return [repo[0] for repo in github_list]

    def _absolute_path(self, name):
        return os.path.join(self.directory, name)

    def _clone(self, name, url):
        target = self._absolute_path(name)
        out, err, ret = self._execute(["git", "clone", url, target])

        # Rather hacky
        if len(err):
            errors = [line.strip() for line in err.split('\n') if len(line)]
            got_error = False
            for error in errors:
                if error != "warning: You appear to have cloned an empty repository.":
                    got_error = True
                    break
            if got_error:
                raise AddonError(name, "Error cloning: " + err)

    def _get_new_addons(self):
        """Check out any new add-ons.

        Returns whether anything changed.
        """
        changed = False
        github_list = self._github_repos_list()
        local_list = self._get_local_addons()
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

    _github_repos_memo = None
    def _github_repos_list(self, readonly=False):
        """Get a list of repositories.

        readonly: Should the tuples have ssh urls or readonly urls.

        Returns a list of tuples that contain the add-on name and the url.
        """
        if not self._github_repos_memo:
            url = _GITHUB_API_BASE + _GITHUB_API_REPOS
            self._github_repos_memo = self._github_api_request(url)

        version_suffix = "-{0}".format(self.version)
        return [(repo["name"][:-len(version_suffix)], repo["git_url"] if readonly else repo["ssh_url"])
                    for repo in self._github_repos_memo if repo["name"].endswith(version_suffix)]

    def _github_repos_create(self, name):
        """Create a new repository.

        name: The name of the add-on for which the repository will be created.
        """
        reponame = "{0}-{1}".format(name, self.version)

        # Create the repository
        url = _GITHUB_API_BASE + _GITHUB_API_REPOS
        requestdata = { "name" : reponame }
        repodata = self._github_api_request(url, requestdata, authenticate=True)

        # Request the teams
        url = _GITHUB_API_BASE + _GITHUB_API_TEAMS
        teams = self._github_api_request(url, authenticate=True)

        # Find the right team number
        # This can probably be cleaner
        team_number = [team["id"] for team in teams if team["name"] == "Developers"][0]

        # Add the repository to the team
        # PUT /teams/:id/repos/:org/:repo
        baseurl = _GITHUB_API_BASE + _GITHUB_API_TEAM_REPO
        url = baseurl.format(team_number, reponame)
        # Github requires data for every modifying request, even if there is none
        self._github_api_request(url, data="", method="PUT", authenticate=True)

        # Add commit hook
        baseurl = _GITHUB_API_BASE + _GITHUB_API_HOOKS
        url = baseurl.format(reponame)
        requestdata = { "name" : "web", "events" : ["push"], "active" : True,
            "config" : {
                "url" : "http://ai0867.net:6660/wescamp",
                "content_type" : "json"
            }
        }
        self._github_api_request(url, requestdata, authenticate=True)

        return repodata

    def _github_api_request(self, url, data=None, method=None, authenticate=False):
        logging.debug("Making github API request {0}".format(url))

        request = urllib.request.Request(url)
        if method:
            request.get_method = lambda: method

        if data == "":
            # Workaround for PUTs requiring data, even if you have nothing to pass
            request.add_data(data)
        elif data:
            request.add_data(json.dumps(data))

        # Manually adding authentication data
        # Basic works in curl, but urllib2 doesn't
        # probably because github's API doesn't send a www-authenticate header
        if authenticate or self._github_have_authorization():
            from base64 import encodestring
            auth = self._github_authorization()
            if ":" in auth:
                # username:password
                base64string = encodestring(auth).replace('\n', '')
                request.add_header("Authorization", "Basic {0}".format(base64string))
            else:
                # token
                request.add_header("Authorization", "Bearer {0}".format(auth))

        try:
            response = urllib.request.urlopen(request)
        except IOError as e:
            raise Error("GitHub API failure: " + str(e))
        if response.code == 204:
            # 204 = No content
            return None

        json_parsed = json.load(response)

        link_headers = response.info().getallmatchingheaders("Link")
        if link_headers:
            logging.debug("Found a Link header in response, analyzing...")
            link_header = link_headers[0].lstrip("Link:")
            links_raw = link_header.split(",")
            links_split_raw = [link.split(";") for link in links_raw]
            links_split_proc = [(l[1].strip().lstrip('rel="').rstrip('"'), l[0].strip().lstrip("<").rstrip(">")) for l in links_split_raw]
            links_dict = dict((k, v) for (k, v) in links_split_proc)
            if "next" in links_dict:
                logging.debug("Link with rel=\"next\" found, recursing to deal with pagination")
                rest = self._github_api_request(links_dict["next"], data, method, authenticate)
                json_parsed += rest

        return json_parsed

    def _github_have_authorization(self):
        return self.authorization != None
    def _github_authorization(self):
        if self.authorization:
            return self.authorization
        else:
            raise Error("Authentication required")

    def _execute(self, command, cwd=None, check_error=False):
        #TODO: have an errorcheck that actually checks the returncode?
        """Executes a command.

        command: The command to execute.
        cwd: Directory to execute the command from.
        check_error: Whether to raise an exception if there's stderr output.
        Returns stdout, stderr.
        Raises libgithub.Error if check_error and len(err).
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

        logging.debug("===== stdout ====\n%s\n===== stdout ====", out)
        logging.debug("===== stderr ====\n%s\n===== stderr ====", err)

        if check_error and len(err):
            raise Error("Failure executing command '{0}': {1}".format(" ".join(command), err))

        return _execresult(out, err, p.returncode)

def _gen(possible_dirs):
    def _get_build_system(possible_dirs):
        logging.debug("get_build_system with paths: %s", ";".join(possible_dirs))

        if not isinstance(possible_dirs, list):
            raise Error("Incorrect argument type passed, {0} instead of {1}".format(str(type(possible_dirs)), str(list)))

        def is_good_checkout(addon):
            try:
                out, err, ret = addon._execute(["git", "remote", "-v"], check_error=True)
                test = "wescamp/build-system"
                return test in out
            except:
                return False

        for path in possible_dirs:
            base, rest = os.path.split(path.rstrip(os.sep))
            fake_github = GitHub(base, "system")
            fake_build = Addon(fake_github, rest, True)
            if is_good_checkout(fake_build):
                logging.debug("Found {0} to be valid build-system checkout".format(path))
                return fake_build, False
            else:
                logging.debug("Discarded possible checkout {0}".format(path))

        logging.debug("No candidates left, creating new checkout")

        realish_github = GitHub(tempfile.mkdtemp(), "system")
        build_system = realish_github.addon("build", readonly=True)
        return build_system, True
    try:
        bs, fresh = _get_build_system(possible_dirs)
        bs.update()
    except Error as e:
        # Exception to make sure nobody catches it
        # Use raise ... from syntax in python3
        import sys
        raise Exception(str(e)).with_traceback(sys.exc_info()[2])
    # Add references to shutil and os to ensure we're destructed before they are
    stored_shutil = shutil
    stored_os = os
    try:
        while True:
            # Don't make a fresh clone every call
            yield bs
    except GeneratorExit:
        # Clean up our temporary clone
        if fresh:
            stored_shutil.rmtree(bs.get_dir())
            stored_os.rmdir(os.path.dirname(bs.get_dir()))

_g = None
def get_build_system(possible_dirs=[]):
    """Create a special 'add-on', containing the wescamp build system.

    possible_dirs: List of paths to possible existing.
    Returns: The Addon object of the build-system
    """
    global _g
    if _g == None:
        _g = _gen(possible_dirs)
    return next(_g)
