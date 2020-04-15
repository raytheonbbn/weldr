#!/bin/bash
#Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.

FILE=$1
SYM_LIST=$2

#Reset the ELF file type to relocatable.
printf "\x01" | dd of=$FILE bs=1 seek=16 count=1 conv=notrunc

#Remove unwanted sections from the file
objcopy -R "*" -R "!.text" -R "!.bss" -R "!.comment" -R "!.data" -R "!.eh_frame" $FILE

#Remove unwanted symbols from the file
objdump -t $FILE | grep -oE "\S+$" > tmp_syms.txt
while read LINE; do
    grep "$LINE" $SYM_LIST > /dev/null
    IS_SYM="$?"
    echo "$LINE" | grep -E "^\.\S+$" > /dev/null
    IS_HEADER="$?" 
    echo "$LINE" | grep -E "\S+\.c$" > /dev/null
    IS_FILENAME="$?"

    if [[ "$IS_SYM" == "1" && "$IS_HEADER" == "1" && "$IS_FILENAME" == "1" ]]; then
        objcopy -N "$LINE" $FILE
    fi
done < "tmp_syms.txt"

rm -f tmp_syms.txt
