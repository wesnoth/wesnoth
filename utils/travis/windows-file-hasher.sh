#!/bin/bash
DBFILE="$1"
if [ "$DBFILE" == "" ]; then
  echo "No database file provided!"
  exit 1
fi

sqlite3 $DBFILE "create table if not exists FILES(NAME VARCHAR(1000) PRIMARY KEY, MD5 VARCHAR(32), OLD_MD5 VARCHAR(32) DEFAULT '-')"

for file in $(find . -type f | grep '\.c$\|\.cpp$\|\.h$\|\.hpp$\|\.ipp$\|\.tpp$\|\.ii' | cut -c 3-); do
  newhash=`md5sum "$file" | cut -d" " -f1`
  oldhash=`sqlite3 "$DBFILE" "select MD5 from FILES where NAME = '"$file"'"`

  if [ "$oldhash" == "" ]; then
    sqlite3 "$DBFILE" "insert into FILES(NAME, MD5) values('$file', '$newhash')"
    printf '%-15s %-60s has %-32s\n' "new file:" "$file" "$newhash"
  elif [ "$oldhash" != "$newhash" ]; then
    sqlite3 "$DBFILE" "update FILES set OLD_MD5 = MD5, MD5 = '$newhash' where NAME = '$file'"
    printf '%-15s %-60s had %-32s %-13s %-32s\n' "changed file:" "$file" "$oldhash" "and now has" "$newhash"
  else
    printf '%-15s %-60s had %-32s %-13s %-32s\n' "unchanged file:" "$file" "$oldhash" "and still has" "$newhash"
    touch -d "20 years ago" "$file"
  fi
done

for file in `sqlite3 "$DBFILE" "select NAME from FILES"`; do
  file=`echo "$file" | tr -d '\r'`
  if [ ! -f "$file" ]; then
    printf '%-15s %-60s\n' "File removed:" "$file"
    sqlite3 "$DBFILE" "delete from FILES where NAME = '$file'"
  fi
done
