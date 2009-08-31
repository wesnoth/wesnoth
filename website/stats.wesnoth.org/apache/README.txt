#Here is a numbered list of what you need to do to deploy you Turbogears2 or Pylons application. Follow 1 through 6. 

#1. Create production.ini configuration file if its not there already. Example: 
cp development.ini production.ini
#Edit production.ini and delete the port settings or make sure its set to 80.

#2. Change or check the apache settings file.
#Edit /usr/local/turbogears/wesstats/apache/wesstats and make sure it has the necessary apache configurations you need.
#Copy {wesstats} apache config file to apache folder. Example:
cp /usr/local/turbogears/wesstats/apache/wesstats /etc/apache2/sites-available/wesstats

#3.Check if permissions are the same as other apache sites usually (root:root)

ls -l /etc/apache2/sites-available/
#You shoud see
#total 16
#-rw-r--r-- 1 root root  950 2008-08-08 13:06 default
#-rw-r--r-- 1 root root 7366 2008-08-08 13:06 default-ssl
#-rw-r--r-- 1 root root 1077 2008-11-08 12:38 wesstats

#4.Enable your site.
a2ensite wesstats

#5. Check if your project has proper permissions, usually apache user. (Example: www-data:www-data on Debian).
ls -l /usr/local/turbogears/wesstats/apache/
#total 16
#-rw-r--r-- 1 www-data www-data 1077 2008-11-26 22:35 wesstats
#-rw-r--r-- 1 www-data www-data 2319 2008-11-26 23:25 wesstats.wsgi
#-rw-r--r-- 1 www-data www-data  594 2008-11-26 22:35 README.txt
#-rw-r--r-- 1 www-data www-data  538 2008-11-26 22:35 test.wsgi

#6.Reload apache
/etc/init.d/apache2 reload


#You are done. Your application should be working. Check the access.log, warn.log, and error.log in /var/log/apache to see if there are any errors. 
