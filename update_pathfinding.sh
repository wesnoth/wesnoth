#!/bin/bash

for i in {1..7}; do
  ./wesnoth --nocache --log-debug=wml --noreplaycheck --unit characterize_pathfinding_reach_${i} 2> ${i}.txt
  cat ${i}.txt | sed -n 's/^.*: \([ [:digit:]]*\) ,/\1/p' | tr '\n' ',' > ${i}_answers.txt
  rm ${i}.txt
done
