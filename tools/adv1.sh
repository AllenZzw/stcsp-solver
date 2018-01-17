#!/bin/bash
rm solutions.dot
./stcsp -a -s $1
python ./find_path.py solutions.dot
