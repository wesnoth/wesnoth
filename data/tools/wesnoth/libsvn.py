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
    def __init__(self, checkout, verbose = False):
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

        self.verbose = verbose
        self.out = ""
        self.err = ""
    

    """V makes a new checkout
    
    repo the repo to checkout eg http://svn.gna.org/svn/wesnoth/trunk 
    returns a result object (note if an checkout was already there it always returns 1
    no indication whether something is updated)
    """
    def checkout(self, repo):

        out, err = self.execute("svn co --non-interactive " + repo + " " + self.checkout_path)

        if(err != ""):
            return result(-1, out, err)

        return result(1, out)


    """V commits the changes

    after deleting a local file and committing that change the file remains
    we also clean those files
    msg is the commit message
    files optional list with files/directories to check in if ommitted
    all modifications are send.
    returns a result object
    """
    def commit(self, msg, files = None):
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

    """T updates the local checkout

    rev revision to update to
    files optional list of files to update
    returns a result object, if no files are changed with the update the
    result is 0. After the update the status if the confilicting files is checked
    conflicts in properties is ignored (note both should never happen not even sure
    we should test it... for now ignore it we'll fail on an update)
    """
    def update(self, rev = None, files = None):
        command = "svn up --non-interactive "
        if(rev != None):
            command += "-r " + rev + " "
        if(files != None):
            command += files

        # execute
        out, err = self.execute(command + self.checkout_path)

        if(err != ""):
            return result(-1, out, err)

        return result(1, out)


    """T syncs local files into a local checkout adds new files and removes files
    not in the local files and updates the others.

    sync_dir to with files to sync with
    exclude files in the checkout directory which should not be touched

    returns a result object
    """
    def sync(self, src, exclude = None):

        # check whether the status of the repo is clean
        out, err = self.execute("svn st " + self.checkout_path)

#        print ("root = " + self.checkout_path + "\nsync_dir = " + src)

        if(err != ""):
            return result(-1, out, err)
        elif(out != ""):
            return result(-1, out, "checkout not clean:\n" + out)


        return self.sync_dir(src, self.checkout_path, exclude)


    def sync2(self, sync_dir, exclude = None):

        # check whether the status of the repo is clean
        out, err = self.execute("svn st " + self.checkout_path)

        print ("root = " + self.checkout_path + "\nsync_dir = " + sync_dir)


        if(err != ""):
            return result(-1, out, err)
        elif(out != ""):
            return result(-1, out, "checkout not clean:\n" + out)


        # check for files in the checkout but not in the sync dir
        # these files should be removed
        print "REMOVE"
        out_result = ""
        base_len = len(self.checkout_path)
        for root, dirs, files in os.walk(self.checkout_path):
            # ignore the .svn dirs
            if '.svn' in dirs:
                dirs.remove('.svn')  

            # first remove directories, since files are handled recursively
            for dir in dirs:
                print "walked into dir: " + dir

                # is the directory in the exclude list? FIXME implant
                if(False):
                    continue


                # if the directory doesn't exist remove it and don't
                # walk further into it
                if(not(os.path.isfile(sync_dir + dir))):
                    print "removing dir: " + root + "/" + dir
                    res = self.remove(root + "/" + dir)
                    out_result += res.out
                    if(res.status == -1):
                        return result(-1, out_result, res.err)
                    dirs.remove(dir)

            # now test the files
            for file in files:

                dir = root[base_len:]
                if(dir != ""):
                    dir += "/"

                print "walked into file: " + dir + " " + file

                # is the file in the exclude list? FIXME implant
                if(False):
                    continue

                # if the file doesn't exist remove it
                if(not(os.path.isfile(sync_dir + dir + file))):
                    print "removing file: " + root + "/" + file
                    out, err = self.remove(root + "/" + file)
                    out_result += out
                    if(err != ""):
                        return result(-1, out_result, err)

        # check for files in the sync dir but not in the checkout
        # these files should be added
        print "ADD"
        base_len = len(sync_dir)
        add_list = []
        for root, dirs, files in os.walk(sync_dir):
            # ignore the .svn dirs
            if '.svn' in dirs:
                dirs.remove('.svn')  

            # first add directories, since files are handled recursively
            for dir in dirs:
                print "walked into dir: " + dir

                # is the directory in the exclude list? FIXME implant
                if(False):
                    continue


                # if the directory doesn't exist add it and don't
                # walk further into it
                if(not(os.path.isfile(self.checkout_path + dir))):
                    print "adding dir: " + root + "/" + dir
                    add_list.append(self.checkout_path + dir)
                    dirs.remove(dir)

            # now test the files
            for file in files:

                dir = root[base_len:]
                if(dir != ""):
                    dir += "/"

                print "walked into file: " + dir + " " + file

                # is the file in the exclude list? FIXME implant
                if(False):
                    continue

                # if the file doesn't exist add it
                if(not(os.path.isfile(self.checkout_path + dir + file))):
                    print "adding file: " + root + "/" + file
                    add_list.append(self.checkout_path + dir + file)


        # copy the files from the sync dir to the checkout
        print "COPY"
        base_len = len(sync_dir)
        for root, dirs, files in os.walk(sync_dir):
            # ignore the .svn dirs
            if '.svn' in dirs:
                dirs.remove('.svn')  

            # first add directories, since files are handled recursively
            for dir in dirs:
                print "walked into dir: " + dir

                # is the directory in the exclude list? FIXME implant
                if(False):
                    continue


                # if the directory doesn't exist add it
                if(not(os.path.isfile(self.checkout_path + dir))):
#                    print "creating dir: " + root + "/" + dir
#                    os.mkdir(root + "/" + dir)
                    
                    print "creating dir: " + self.checkout_path + "/" + dir
                    os.mkdir(self.checkout_path + "/" + dir)

            # now test the files
            for file in files:

                dir = root[base_len:]
                if(dir != ""):
                    dir += "/"

                print "walked into file: " + dir + " " + file

                # is the file in the exclude list? FIXME implant
                if(False):
                    continue

                # copy the file
                shutil.copy(root + file, self.checkout_path + dir + file)

        print "ADD REALLY this time"
        for file in add_list:
            res = self.add(file)
            out_result += res.out
            if(res.status == -1):
                return result(-1, out_result, res.err)

        print "DONE"
        return result(1, out_result)


    def sync_dir(self, src, dest, exclude = None):

#        print "Syncing dir " + src + " and " + dest

        src_dirs, src_files = self.get_dir_contents(src, exclude)
        dest_dirs, dest_files = self.get_dir_contents(dest, exclude)

        # If a directory exists in the src but not in the dest, the entire
        # thing needs to be copied recursively.

        # If a directory doesn't exist in the src but does in the dest, the
        # entire thing needs to be deleted recursively.

        # If a directory exists in both, it needs to be scanned recursively.


        for dir in src_dirs:
#            print "Testing directory " + dest + "/" + dir
            if(os.path.isdir(dest + "/" + dir) == False):
                # src only
#                print ("> not found")
                res = self.dir_add(src + "/" + dir, dest + "/" + dir)
                if(res.status == -1):
                    return res
            else:
                # in both
#                print ("> found")
                res = self.sync_dir(src + "/" + dir, dest + "/" + dir, exclude)
                if(res.status == -1):
                    return res

        for dir in dest_dirs:
#            print "Testing directory " + src + "/" + dir
            if(os.path.isdir(src + "/" + dir) == False):
                # dest only
#                print ("> not found")
                res = self.dir_remove(dest + "/" + dir)
                if(res.status == -1):
                    return res
            else:
                # in both
#                print ("> found")
                pass
                
        # If a file exists in the src but not in the dest, it needs to be copied.

        # If a file doesn't exist in the src but does in the dest, it needs to be 
        # deleted.

        # If a file exists in both it needs to be copied.

        for file in src_files:
#            print "Testing file" + dest + "/" + file
            if(os.path.isfile(dest + "/" + file) == False):
                # src only
#                print ("> not found")
                res = self.file_add(src + "/" + file, dest + "/" + file)
                if(res.status == -1):
                    return res
            else:
                # in both
#                print ("> found")
                res = self.file_copy(src + "/" + file, dest + "/" + file)
                if(res.status == -1):
                    return result

        for file in dest_files:
#            print "Testing file" + src + "/" + file
            if(os.path.isfile(src + "/" + file) == False):
                # dest only
#                print ("> not found")
                res = self.file_remove(dest + "/" + file)
                if(res.status == -1):
                    return res
            else:
                # in both
#                print ("> found")
                pass

        # FIXME we didn't accumulate the output
        return result(1)


    """\ Foo
    """
    def get_dir_contents(self, dir, exclude = None):
#        if(self.verbose):
#            print("walked into directory " + dir)

        items = os.listdir(dir)
        dirs = []
        files = []

        for item in items:

            # ignore .svn dirs
            if(item == '.svn'):
                continue

            # FIXME ignore exclude list
            

            # an item is either a directory or not, in the latter case it's
            # assumed to be a file.
            if(os.path.isdir(dir + "/" + item)):
                dirs.append(item)
            else:
                files.append(item)

        return dirs, files

    def dir_add(self, src, dest):
        print "Add dir " + src + " to " + dest

        # add parent
        os.mkdir(dest)
        res = self.add(dest)
        if(res.status == -1):
            return res

        # get sub items
        dirs, files = self.get_dir_contents(src)

        # copy files
        for file in files:
            res = self.file_add(src + "/" + file, dest + "/" + file)
            if(res.status == -1):
                return res
           
        # copy dirs
        for dir in dirs:
            res = self.dir_add(src + "/" + dir, dest + "/" + dir)
            if(res.status == -1):
                return res

        return result(1)

    def dir_remove(self, dir):
#        print "Remove dir " + dir
        return result(1)

    """ FIXME we assume we copy to svn 
    """
    def file_add(self, src, dest):
#        print "Add file " + src + " to " + dest
        shutil.copy(src, dest)
        return self.add(dest)

    def file_copy(self, src, dest):
#        print "Copy file " + src + " to " + dest
        shutil.copy(src, dest)
        return result(1)


    """ FIXME we assume we remove from svn 
    """
    def file_remove(self, file):
#        print "Remove file " + file
        return self.remove(file)

    """T adds a file to the repo

    note adding an exisiting file is an error now 
    should return 0 do nothing

    NOTE svn info could do the trick if returns out
    it under control if returns err not under control

    but if scheduled for removal but not commited 
    svn info still returns info :/
    """
    def add(self, file):

        # FIXME  we should test whether the file is already in the repo
        
        # execute (repo not required)
        out, err = self.execute("svn add " + file)

        if(err != ""):
            return result(-1, out, err)

        return result(1, out)

    
    """T removes a file from the repo

    """
    def remove(self, file):
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
    def status(self, file, mask = None):
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

    """X Tests whether a file in the local checkout exists (private)
    
    returns True of False
    """
    def file_exists(self, file):
        return result(-1, "", "not implanted")

    """V Executes the command (private)
    
    returns stdout, stderr
    """
    def execute(self, command):
        
        if(self.verbose):
            print "Execute: " +  command

        stdin, stdout, stderr =  os.popen3(command)
        stdin.close()
        err = stderr.read()
        stderr.close()
        out = stdout.read()
        stdout.close()

        return out, err

