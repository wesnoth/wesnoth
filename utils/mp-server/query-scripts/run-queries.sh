#!/bin/sh

for sql in ./*.sql
do
    mysql < "$sql" > "${sql}.tsv"
done
