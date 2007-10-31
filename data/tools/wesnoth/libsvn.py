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

import os, shutil;

class result:
    """V Result object for most svn functions

    status  -1 error occured err has the message
             0 nothing to do (command might be executed)
             1 success
    out     output of the svn command
    err     error of the svn command
    """
    def __init__(self, status, out = "", err = ""):
        self.status = status
        self.out = out
        self.err = err

class SVN:
    """\ Initializes a SVN object
    
    checkout the root of the local checkout eg /src/wesnoth
    do not include a trailing slash!
    """
    def __init__(self, checkout, log_level = 1):
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

    
        self.LOG_LEVEL_ERROR = 0
        self.LOG_LEVEL_WARNING = 1
        self.LOG_LEVEL_INFO = 2
        self.LOG_LEVEL_DEBUG = 3

        self.log_level = log_level
        self.out = ""
        self.err = ""
    

    """V Makes a new checkout.
    
    repo                The repo to checkout eg 
                        http://svn.gna.org/svn/wesnoth/trunk
    returns             A result object (note if an checkout was already there
                        it always returns 1 no indication whether something is 
                        updated).
    """
    def svn_checkout(self, repo):

        self.log(self.LOG_LEVEL_DEBUG, "checkout " + repo)

        out, err = self.execute("svn co --non-interactive " + repo + " " + 
            self.checkout_path)

        if(err != ""):
            return result(-1, out, err)

        return result(1, out)


    """V Commits the changes

    After deleting a local file and committing that change the file remains.
    msg                 The commit message.
    files               Optional list with files/directories to check in if 
                        ommitted all modifications are send.
    returns             A result object.
    """
    def svn_commit(self, msg, files = None):

        self.log(self.LOG_LEVEL_DEBUG, "commit msg " + msg)

        command = "svn commit --non-interactive -m " + '"' + msg + '"'
        if(files != None):
            command += " " + files

        # execute
        out, err = self.execute(command + " " + self.checkout_path)

        if(err != ""):
            return result(-1, out, err)
        elif(out != ""):
            return result(1, out)
        else:
            # no output nothing committed
            return result(0, out)

    """V updates the local checkout

    rev                 Revision to update to, if ommitted updates to HEAD.
    files               Optional list of files to update, if ommitted the
                        checkout is updated.
    returns             A result object, returns 0 if no changes were made.
    """
    def svn_update(self, rev = None, files = None):
        command = "svn up --non-interactive "
        if(rev != None):
            command += "-r " + rev + " "
        if(files != None):
            command += files

        # execute
        out, err = self.execute(command + self.checkout_path)

        if(err != ""):
            return result(-1, out, err)

        if(out.count('\n') == 1):
            return result(0, out)

        return result(1, out)

    """T Copies local files to an svn checkout.
    
    src                 Directory with the source files.
    exclude             List with names to ignore.
    returns             A result object, returns 0 if no changes are made after 
                        the copy operation.
    """
    def copy_to_svn(self, src, exclude):# = None):

        self.log(self.LOG_LEVEL_DEBUG, "copy_to_svn :\n\tsvn = " 
            + self.checkout_path + "\n\tsrc = " + src)

        # check whether the status of the repo is clean
        out, err = self.execute("svn st " + self.checkout_path)
        
        # if not clean or an error bail out
        if(err != ""):
            return result(-1, out, err)
        elif(out != ""):
            return result(-1, out, "checkout not clean:\n" + out)

        # update
        res = self.sync_dir(src, self.checkout_path, False, exclude)

        # Only if if the status is 1 it might change to 0
        if(res.status != 1) :
            return res

        # no error, test whether clean or not, if clean we need to set the
        # status to 0
        out, err = self.execute("svn st " + self.checkout_path)
        if(out == ""):
            res.status = 0
        return res

    """T Syncronizes two directories.

    src                 The source directory.
    dest                The destination directory.
    src_svn             Either the source or the target is a svn checkout
                        if True the source is, else the destination.
    exclude             List with names to ignore.
    returns             A result object, 0 if nothing has been done, if 
                        something is copied it returns 1 and doesn't check
                        whether source and target file are different.
    """
    def sync_dir(self, src, dest, src_svn, exclude ):#= None):

        self.log(self.LOG_LEVEL_DEBUG, "sync_dir :\n\tsrc = " 
            + src + "\n\tdest = " + dest)

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
                res = self.dir_add(src + "/" + dir, dest + "/" + dir, src_svn, exclude)
                if(res.status == -1):
                    return res
            else:
                # in both
                res = self.sync_dir(src + "/" + dir, dest + "/" + dir, src_svn, exclude)
                if(res.status == -1):
                    return res

        for dir in dest_dirs:
            if(os.path.isdir(src + "/" + dir) == False):
                # dest only
                res = self.dir_remove(dest + "/" + dir, not(src_svn))
                if(res.status == -1):
                    return res
                
        # If a file exists in the src but not in the dest, it needs to be copied.

        # If a file doesn't exist in the src but does in the dest, it needs to be 
        # deleted.

        # If a file exists in both it needs to be copied.

        for file in src_files:
            if(os.path.isfile(dest + "/" + file) == False):
                # src only
                res = self.file_add(src + "/" + file, dest + "/" + file, src_svn)
                if(res.status == -1):
                    return res
            else:
                # in both
                res = self.file_copy(src + "/" + file, dest + "/" + file)
                if(res.status == -1):
                    return result

        for file in dest_files:
            if(os.path.isfile(src + "/" + file) == False):
                # dest only
                res = self.file_remove(dest + "/" + file, not(src_svn))
                if(res.status == -1):
                    return res

        # FIXME we didn't accumulate the output
        return result(1)


    """V Gets a list with files and directories.

    The function always ignores .svn entries. Items which aren't a directory 
    are assumed a file.
    dir                 The directory to get the info from.
    exclude             List with names to ignore.
    returns             A list with directories and a list with files.
    """
    def get_dir_contents(self, dir, exclude ):#= None):

        self.log(self.LOG_LEVEL_DEBUG, "get dir :\n\tdir = " + dir)
        if(exclude != None):
            self.log(self.LOG_LEVEL_DEBUG, "\t exclude = ")
            self.log(self.LOG_LEVEL_DEBUG, exclude)

        items = os.listdir(dir)
        dirs = []
        files = []

        for item in items:
            self.log(self.LOG_LEVEL_DEBUG, "\tTesting item " + item)

            # ignore .svn dirs
            if(item == ".svn"):
                self.log(self.LOG_LEVEL_DEBUG, "\t\tIgnore .svn")
                continue

            # ignore exclude list
            if(exclude != None and item in exclude):
                self.log(self.LOG_LEVEL_DEBUG, "\t\tIgnore on the exclude list")
                continue
            
            # an item is either a directory or not, in the latter case it's
            # assumed to be a file.
            if(os.path.isdir(dir + "/" + item)):
                self.log(self.LOG_LEVEL_DEBUG, "\t\tAdded directory")
                dirs.append(item)
            else:
                self.log(self.LOG_LEVEL_DEBUG, "\t\tAdded file")
                files.append(item)

        return dirs, files

    """T creates a duplicate of a directory.

    The destination directory shouldn't exist.
    src                 The source directory.
    dest                The destination directory.
    src_svn             Either the source or the target is a svn checkout
                        if True the source is, else the destination.
    exclude             List with names to ignore.
    returns             A result object, 0 if nothing has been done, if 
                        something is copied it returns 1 and doesn't check
                        whether source and target file are different.
    """
    def dir_add(self, src, dest, src_svn, exclude ):#= None):

        self.log(self.LOG_LEVEL_DEBUG, "dir_add :\n\tsvn = " 
            + self.checkout_path + "\n\tsrc = " + src)

        # add parent
        os.mkdir(dest)
        if(src_svn == False):
            res = self.svn_add(dest)
            if(res.status == -1):
                return res

        # get sub items
        dirs, files = self.get_dir_contents(src, exclude)

        # copy files
        for file in files:
            res = self.file_add(src + "/" + file, dest + "/" + file, src_svn)
            if(res.status == -1):
                return res
           
        # copy dirs
        for dir in dirs:
            res = self.dir_add(src + "/" + dir, dest + "/" + dir, src_svn, exclude)
            if(res.status == -1):
                return res

        return result(1)

    """ FIXME IMPLEMENT 
    """
    def dir_remove(self, dir):
        return result(1)

    """T Adds a file.

    If src_svn is True it does the same as copy file.
    src                 The source directory.
    dest                The destination directory.
    src_svn             Either the source or the target is a svn checkout
                        if True the source is, else the destination.
    returns             A result object.
    """
    def file_add(self, src, dest, src_svn):
        shutil.copy(src, dest)

        if(src_svn):
            return (1)
        else:
            return self.svn_add(dest)

    """T Copies a file.

    src                 The source directory.
    dest                The destination directory.
    returns             A result object.
    """
    def file_copy(self, src, dest):
        shutil.copy(src, dest)
        return result(1)

    """ T Removes a file

    file                The file to remove.
    is_svn              Is the file in an svn checkout.
    returns             A result object.
    """
    def file_remove(self, file, is_svn):
        if(is_svn):
            return self.svn_remove(file)
        else:
            os.remove(file)
            return result(1)

    """T adds a file to the repo

    note adding an exisiting file is an error now 
    should return 0 do nothing

    NOTE svn info could do the trick if returns out
    it under control if returns err not under control

    but if scheduled for removal but not commited 
    svn info still returns info :/
    """
    def svn_add(self, file):

        # FIXME  we should test whether the file is already in the repo
        
        # execute (repo not required)
        out, err = self.execute("svn add " + file)

        if(err != ""):
            return result(-1, out, err)

        return result(1, out)

    
    """T removes a file from the repo

    """
    def svn_remove(self, file):
        # FIXME  we should test whether the file is in the repo

        # execute (repo not required)
        out, err = self.execute("svn remove --non-interactive " + file)

        if(err != ""):
            return result(-1, out, err)

        return result(1, out)


    """\ gets the status from a file in the repo 
    or a mask of all stausses
    

    """
    # note the mask should be send by reference
    def svn_status(self, file, mask = None):
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

    """\ Cleans up a checkout

    After a commit where a directory is removed the client doesn't remove
    this directory. This routine removes all files with the '?' flag.
    returns             A result object.
    """
    def svn_cleanup(self):
        # FIXME do something
        pass

    """V Executes the command (private)
    
    returns stdout, stderr
    """
    def execute(self, command):
        
        self.log(self.LOG_LEVEL_DEBUG, "Execute: " + command)

        stdin, stdout, stderr =  os.popen3(command)
        stdin.close()
        err = stderr.read()
        stderr.close()
        out = stdout.read()
        stdout.close()

        return out, err

    def log(self, level, msg):
        if(level <= self.log_level):
            print msg

