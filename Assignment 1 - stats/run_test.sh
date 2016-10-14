#!/bin/bash
sum=32
count=6
echo "(${sum}/${count})" | bc -l
awk_proper_rounding='
    {printf("%d\n",$1 + 0.5)}
'

echo "(${sum}/${count})" | bc -l | awk "${awk_proper_rounding}"