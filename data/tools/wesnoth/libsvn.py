# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
# $Id$ 
"""
   Copyright (C) 2007 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
"""   

"""
This library provides an interface to svn, the interface is build upon
the command line svn tool. 
"""

# TODO make private methods
# public
#   checkout
#   update
#   add
#   remove
#   commit
#   sync
# the others private, public methods send relative items
# for usage, private use the full path


import os, shutil, logging;

class error(Exception):
    """Base class for exceptions in this module."""
    pass


class SVN:
    """Initializes a SVN object.
    
    Checkout the root of the local checkout eg /src/wesnoth
    do not include a trailing slash!
    """
    def __init__(self, checkout):

        logging.debug("SVN constructor path = '%s'", checkout)

        self.checkout_path = checkout

        """status masks
        A = add
        D = delete
        M = modifed
        C = confict
        ? = not under version control
        X = doesn't exist
        """
        self.STATUS_FLAG_ADD = 0x01
        self.STATUS_FLAG_DELETE = 0x02
        self.STATUS_FLAG_MODIFY = 0x04
        self.STATUS_FLAG_CONFLICT = 0x08
        self.STATUS_FLAG_NON_SVN = 0x10
        self.STATUS_FLAG_NON_EXISTANT = 0x20

    """Makes a new checkout.
    
    repo                The repo to checkout eg 
                        http://svn.gna.org/svn/wesnoth/trunk.
    returns             Nothing.
    """
    def svn_checkout(self, repo):

        logging.debug("checkout repo = '%s'", repo)

        out, err = self.execute("svn co --non-interactive " + repo + " " + 
            self.checkout_path)

        if(err != ""):
            raise error("checkout failed with message:" + err)

        logging.debug("checkout output:" + out)

    """Commits the changes.

    After deleting a local file and committing that change the file remains.
    msg                 The commit message.
    files               Optional list with files/directories to commit if 
                        ommitted all modifications are send.
    returns             True if committed, False if nothing to commit.
    """
    def svn_commit(self, msg, files = None):

        logging.debug("commit msg = '%s' files = '%s' ", msg, files)

        command = "svn commit --non-interactive -m " + '"' + msg + '"'
        if(files != None):
            command += " " + files

        # execute
        out, err = self.execute(command + " " + self.checkout_path)

        if(err != ""):
            raise error("commit failed with message:" +err)
        elif(out != ""):
            logging.debug("commit output:" + out)
            return True
        else:
            # no output nothing committed
            logging.debug("commit has no output")
            return False

    """Updates the local checkout.

    rev                 Revision to update to, if ommitted updates to HEAD.
    files               Optional list of files to update, if ommitted the
                        checkout is updated.
    returns             True if update changed files, False otherwise.
    """
    def svn_update(self, rev = None, files = None):

        logging.debug("update rev = '%s' files = '%s'", rev, files)

        command = "svn up --non-interactive "
        if(rev != None):
            command += "-r " + rev + " "
        if(files != None):
            command += self.checkout_path + "/" + files
        else:
            command += self.checkout_path

        # execute
        out, err = self.execute(command)

        if(err != ""):
            raise error("update failed with message:" +err)

        if(out.count('\n') == 1):
            logging.debug("update didn't change anything")
            return False

        logging.debug("update output:" + out)
        return True

    """Copies local files to an svn checkout.
    
    src                 Directory with the source files.
    exclude             List with names to ignore.
    returns             True if the copy resulted in a modified checkout, 
                        False otherwise.
    """
    def copy_to_svn(self, src, exclude):

        logging.debug("copy to svn src = '%s' exclude = '%s'",
            src, exclude)

        # Check whether the status of the repo is clean.
        out, err = self.execute("svn st " + self.checkout_path)
        
        # If not clean or an error bail out.
        if(err != ""):
            raise error("status failed with message:" + err)
        elif(out != ""):
            raise error("checout is not clean:" + out)

        # Update.
        self.sync_dir(src, self.checkout_path, False, exclude)

        # Test whether the checkout is clean if not something is modified.
        # An error shouldn't occur, since we tested that before.
        out, err = self.execute("svn st " + self.checkout_path)
        return (out != "")

    """Syncronizes two directories.

    src                 The source directory.
    dest                The destination directory.
    src_svn             Either the source or the target is a svn checkout
                        if True the source is, else the destination.
    exclude             List with names to ignore.
    returns             Nothing.
    """
    def sync_dir(self, src, dest, src_svn, exclude ):

        logging.debug("sync_dir src = '%s' dest = '%s'", src, dest)

        # Get the contents of src and dest
        src_dirs, src_files = self.get_dir_contents(src, exclude)
        dest_dirs, dest_files = self.get_dir_contents(dest, exclude)

        # If a directory exists in the src but not in the dest, the entire
        # thing needs to be copied recursively.

        # If a directory doesn't exist in the src but does in the dest, the
        # entire thing needs to be deleted recursively.

        # If a directory exists in both, it needs to be scanned recursively.

        for dir in src_dirs:
            if(os.path.isdir(dest + "/" + dir) == False):
                # src only
                self.dir_add(src + "/" + dir, dest + "/" + dir, src_svn, exclude)
            else:
                # in both
                self.sync_dir(src + "/" + dir, dest + "/" + dir, src_svn, exclude)

        for dir in dest_dirs:
            if(os.path.isdir(src + "/" + dir) == False):
                # dest only
                self.dir_remove(dest + "/" + dir, not(src_svn))
                
        # If a file exists in the src but not in the dest, it needs to be copied.

        # If a file doesn't exist in the src but does in the dest, it needs to be 
        # deleted.

        # If a file exists in both it needs to be copied.

        for file in src_files:
            if(os.path.isfile(dest + "/" + file) == False):
                # src only
                self.file_add(src + "/" + file, dest + "/" + file, src_svn)
            else:
                # in both
                self.file_copy(src + "/" + file, dest + "/" + file)

        for file in dest_files:
            if(os.path.isfile(src + "/" + file) == False):
                # dest only
                self.file_remove(dest + "/" + file, not(src_svn))

    """Gets a list with files and directories.

    The function always ignores .svn entries. Items which aren't a directory 
    are assumed a file.
    dir                 The directory to get the info from.
    exclude             List with names to ignore.
    returns             A list with directories and a list with files.
    """
    def get_dir_contents(self, dir, exclude):

        logging.debug("get dir contents dir = '%s' exclude = '%s'",
            dir, exclude)

        items = os.listdir(dir)
        dirs = []
        files = []

        for item in items:

            # Ignore .svn dirs.
            if(item == ".svn"):
                continue

            # Ignore exclude list.
            if(exclude != None and item in exclude):
                continue
            
            # An item is either a directory or not, in the latter case it's
            # assumed to be a file.
            if(os.path.isdir(dir + "/" + item)):
                dirs.append(item)
            else:
                files.append(item)

        return dirs, files

    """Creates a duplicate of a directory.

    The destination directory shouldn't exist.
    src                 The source directory.
    dest                The destination directory.
    src_svn             Either the source or the target is a svn checkout
                        if True the source is, else the destination.
    exclude             List with names to ignore.
    returns             Nothing.
    """
    def dir_add(self, src, dest, src_svn, exclude):

        logging.debug("dir_add src = '%s' dest = '%s' svn_src = '%s' "
            + "exclude = '%s'", src, dest, src_svn, exclude)

        # add parent
        os.mkdir(dest)
        if(src_svn == False):
            self.svn_add(dest)

        # get sub items
        dirs, files = self.get_dir_contents(src, exclude)

        # copy files
        for file in files:
            self.file_add(src + "/" + file, dest + "/" + file, src_svn)
           
        # copy dirs
        for dir in dirs:
            self.dir_add(src + "/" + dir, dest + "/" + dir, src_svn, exclude)
        
    """ FIXME IMPLEMENT 
    """
    def dir_remove(self, dir):
        logging.debug("dir_remove dir = '%s'", dir)

        raise error("dir_remove not implemented")

    """Adds a file.

    If src_svn is True it does the same as copy file.
    src                 The source directory.
    dest                The destination directory.
    src_svn             Either the source or the target is a svn checkout
                        if True the source is, else the destination.
    returns             Nothing.
    """
    def file_add(self, src, dest, src_svn):
        
        logging.debug("file_add src = '%s' dest = '%s' src_svn = '%s'",
            src, dest, src_svn)

        shutil.copy(src, dest)

        if(src_svn == False):
            self.svn_add(dest)

    """Copies a file.

    src                 The source directory.
    dest                The destination directory.
    returns             Nothing
    """
    def file_copy(self, src, dest):

        logging.debug("file_copy src = '%s' dest = '%s'", src, dest)

        shutil.copy(src, dest)

    """Removes a file

    file                The file to remove.
    is_svn              Is the file in an svn checkout.
    returns             Nothing.
    """
    def file_remove(self, file, is_svn):

        logging.debug("file_remove file = '%s' is_svn = '%s'", file, is_svn)

        if(is_svn):
            self.svn_remove(file)
        else:
            os.remove(file)

    """Add an item to the repo.

    The item can either be a file or directory, if the item is already added
    this operation is a nop.

    item                File or directory to add to svn.
    returns             Nothing.
    """
    def svn_add(self, item):

        logging.debug("svn_add item = '%s'", item)

        # execute (repo not required)
        out, err = self.execute("svn add " + item)

        if(err != ""):
            raise error("svn_add failed with message:" + err)
    
    """Removes an item from the repo.

    If an item is not in the repo it's silently ignored and the
    operation is a nop.

    item                File or directory to remove from svn.
    returns             Nothing.
    """
    def svn_remove(self, item):

        logging.debug("svn_remove item = '%s'", item) #typo fix

        # execute (repo not required)
        out, err = self.execute("svn remove --non-interactive " + item)

        if(err != ""):
            raise error("svn_remove failed with message:" + err)


    """Gets the status of an item."""
    # note the mask should be send by reference
    def svn_status(self, file, mask = None):
    
        # FIXME not used and not tested

        result_mask = 0

        # if a file is send we only look after that file
        # else we test all files in the archive
        files = []
        if(file != None):
            files.append(file)
        else:
            pass

        out = ""
        for file in files:
            o, err = self.execute("svn st " + file  )#+ " " +self.checkout_path)
            out += o + "\n"

            if(err != ""):
                return result(-1, out, err)

            # oke we have a file now test the status
            if(o[0:1] == "A"):
                result_mask += self.STATUS_FLAG_ADD
            if(o[0:1] == "D"):
                result_mask += self.STATUS_FLAG_DELETE
            if(o[0:1] == "M"):# | o[1:2] == "M"):
                result_mask += self.STATUS_FLAG_MODIFY
            if(o[0:1] == "C"):# | o[1:2] == "C"):
                result_mask += self.self.STATUS_FLAG_CONFLICT
            if(o[0:1] == "?"):
                result_mask += self.self.STATUS_FLAG_NON_SVN


        return result(1, out, err)

    """Cleans up a checkout.

    After a commit where a directory is removed the client doesn't remove
    this directory. This routine removes all files with the '?' flag.
    """
    def svn_cleanup(self):
        logging.debug("svn_cleanup")

        raise error("svn_cleanup not implemented")

    """Executes a command.

    command             The command to execute
    returns             stdout, stderr
    """
    def execute(self, command):
        
        logging.debug("execute command = '%s'", command)

        stdin, stdout, stderr = os.popen3(command)
        stdin.close()
        # reading stdout before the stderr seems to avoid 'hanging' not 100% sure
        # why but it might be that the stdout buffer is full and thus everything
        # blocks, so this fix might not be 100% but the documentation doesn't tell
        # much more. All examples on the web don't mention this case as well.
        out = stdout.read()
        stdout.close()
        err = stderr.read()
        stderr.close()

        return out, err
