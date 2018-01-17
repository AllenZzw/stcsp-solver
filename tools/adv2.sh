#!/bin/bash
rm solutions.dot
./stcsp -z -s $1
python ./find_path.py solutions.dot
