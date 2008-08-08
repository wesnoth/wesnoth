ADODB LITE version x.xx

Recommended: You should delete the entire directory for the
older versions of ADOdb Lite when you are upgrading.  This
will remove any files that are nolonger needed by ADOdb Lite.
Leaving the unused files will not cause any problems but it
is best to remove them.

If you are using ADOdb Lite on your website please use the 
thumbnail logo I have provided someone on your main page.

<a href="http://adodblite.sourceforge.net/"><img src="images/adodblite_thumb.jpg" alt="ADOdb Lite Web Site" width="100" height="46" border="0"></a>

We have been using ADODB for a number of our web sites for quite 
some time and decided to look into the memory requirements of the 
latest version.  We were quite frankly horrified to find out ADODB 
uses approximately 700k of ram for each page load.  On some of our 
sites this is 10-50 times larger than the pages of PHP code.  That 
is quite frankly unacceptable.  So we decided to come up with a 
very scaled back ADODB LITE that includes the most commonly used 
functions while being compact.  The current version only uses 
less than 100k of ram.

We are only supporting a subset of the commands from ADODB but 
these are commands most people will be using.  We dropped all of 
the esoteric commands as their usefulness is minimal while they 
consume vast amounts of resources needlessly. We have added a 
new module system that will allow adding new commands and features
to ADOdb Lite.  So many of the ADOdb Commands may eventually be
ported as user selectable modules.

One of the added benifits of ADODB LITE is speed.  It is over 300%
faster than ADOdb.  If you are using a PHP Accelerator like eAccelerator
ADOdb Lite is over 30% faster than ADOdb.  This is a huge improvement 
in speed and also greatly lowers the CPU load.  ADOdb Lite is a better
alternative for site with high traffic.

The currently supported databases are a subset of the databases 
supported by ADODB.

fbsql (FrontBase)
maxdb
msql (MiniSql)
mssql (Microsoft SQL)
mssqlpo (Microsoft SQL Pro)
mysql
mysqli
mysqlt
odbc (Highly experimental and probably full of flaws)
postgres
postgres64
postgres7
postgres8
sqlite
sqlitepo (SQLite Pro)
sybase
sybase_ase

If you are not familier with ADOdb then I would suggest reading the ADOdb 
Manual located at http://phplens.com/lens/adodb/docs-adodb.htm

