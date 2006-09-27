grep -ir profile ../images/| grep -v .svn | cut -f3 -d ' '> /tmp/wesnoth-profile-clean
while read x; do
convert -strip $x $x;
done <  /tmp/wesnoth-profile-clean
cat /tmp/wesnoth-profile-clean
rm /tmp/wesnoth-profile-clean
