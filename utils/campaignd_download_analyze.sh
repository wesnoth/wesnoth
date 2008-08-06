#!/bin/sh
sed -n -e "s/.*campaign list.*[0-9]\( using \(gzip\)\)\? size: \([0-9]*\)kb/\3 \2/p" $1 |awk 'BEGIN { sum = 0; count = 0; gzip = 0 } { sum += $1; count ++; if ($2 == "gzip") gzip ++; } END { print "campaign list sent: " count " gzipped: " gzip " size: " sum "kb" }'
sed -n -e "s/.*campaign '.*[0-9]\( using \(gzip\)\)\? size: \([0-9]*\)kb/\3 \2/p" $1 |awk 'BEGIN { sum = 0; count = 0; gzip = 0 } { sum += $1; count ++; if ($2 == "gzip") gzip ++; } END { print "campaigns sent: " count " gzipped: " gzip " size: " sum "kb" }'
