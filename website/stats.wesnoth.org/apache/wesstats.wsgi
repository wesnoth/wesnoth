#modwsgi script for wesstats

#1.Point to this script in you apache config file.
#Default location for all apps is:
#Debian:  /srv/www/html/ (wesnoth specific)

#2. Make sure apache user own the folder.
#Debian: chown -R www-data:www-data /srv/www/html/wesstats

import sys

#3. start of virtualenv (enabled by default).
#Please comment out until 4 if you don't use virtualenv. 
#Make sure root owns the virtualenv folder. Example:(root:root)
#Create virtualenv if you didn't create it yet:
#mkdir /usr/local/pythonenv
#virtualenv --no-site-packages /usr/local/pythonenv/BASELINE

prev_sys_path = list(sys.path)

import site 
site.addsitedir('/home/corn/tg2env/lib/python2.5/site-packages')

#Move just added item to the front of the python system path. 
#Not needed if modwsgi>=3.0. Uncomment next 6 lines.
new_sys_path = []
for item in list(sys.path):
    if item not in prev_sys_path:
        new_sys_path.append(item)
        sys.path.remove(item)
sys.path[:0] = new_sys_path 

#End of virtualenv

#4. Your website file location.
import os, sys
sys.path.append('/srv/www/html/wesstats')

#5. Set the environment variable PYTHON_EGG_CACHE to an appropriate directory where the Apache user has write permission and into which it can unpack egg files.
os.environ['PYTHON_EGG_CACHE'] = '/srv/www/html/wesstats/python-eggs'

#6.[Optional]If you want to enable logging you need to initialize logging. You also need to setup logger handlers in you production.ini. When done uncomment next two lines.
from paste.script.util.logging_config import fileConfig
fileConfig('/srv/www/html/wesstats/prod.apache.ini')

#7. Load you application production.ini file.
from paste.deploy import loadapp
application = loadapp('config:/srv/www/html/wesstats/prod.apache.ini')


#8.[Optional] If you want to test modwsgi only, uncomment section 3 in you /srv/www/html/wesstats/apache/wesstats
