#!/bin/bash
master_pid=$$
awk_transpose='{
    for ( i=1; i <= NF; i++ )
        row[i] = row[i]((row[i])?" ":"")$i
}

END{
    for ( x = 1; x <= length(row) ; x++ )
        print row[x]
}'

cat $1 | awk "${awk_transpose}" > ${master_pid}"_"transposed
cat ${master_pid}"_"transposed
rm -f ${master_pid}"_"transposed