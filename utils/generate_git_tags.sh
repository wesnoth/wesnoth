#!/bin/bash
for i in `git branch -r -l|grep tags|sed 's#.*tags/##'`;do git tag $i tags/$i -m "SVN tag $i";done

