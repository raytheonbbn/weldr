#!/bin/bash
# Copyright (c) 2019 Raytheon BBN Technologies, Inc. All Rights Reserved.
#
# This document does not contain technology or Technical Data controlled under either
# the U.S. International Traffic in Arms Regulations or the U.S. Export Administration
#
# Distribution A: Approved for Public Release, Distribution Unlimited

#Split args array
#This doesn't use the pythonic negative length,
#because it's not supported in all versions of bash.

SCRIPT_ARGS=()
CMD_ARGS=()
MODE="SCRIPT"

for i
do
    if [ "$MODE" = "SCRIPT" ]; then
        if [ "$i" = "--" ]; then
            MODE="CMD"
        else
            SCRIPT_ARGS+=($i)
        fi
    else
        ARG=$(echo $i | sed "s/ /\\ /g")
        CMD_ARGS+=("$ARG")
    fi
done
    


#Parse the args to this script.
CMD="gcc"
OUTDIR="."

KEY=""
for i in "${!SCRIPT_ARGS[@]}"
do
    if [ "$KEY" = "" ]; then
        KEY=${SCRIPT_ARGS[$i]}
    else
        case $KEY in
            -c|--command)
            CMD=${SCRIPT_ARGS[$i]}
            KEY=""
            ;;
            -o|--outdir)
            OUTDIR=${SCRIPT_ARGS[$i]}
            KEY=""
            ;;
        esac
    fi
done

# Ensure debugging is enabled in EVERYTHING we compile.
echo "${CMD_ARGS[@]}" | grep -- '-g' >/dev/null
if [ "$?" != "0" ]; then
    CMD_ARGS=("-g" "${CMD_ARGS[@]}")
fi

echo "Executing: $CMD ${CMD_ARGS[@]}"
#Check if this is an object file, a source file, or an executable/library
IS_OBJ=$(echo "${CMD_ARGS[@]}" | grep -cE -- "(^|\s)-c")

if [ "$IS_OBJ" = "0" ]; then
    echo "Command is not building an object."
    # Parse the commands to find the output file name.
    EXE_FILE=$(echo "${CMD_ARGS[@]}" | grep -o -- "-o \S*" | cut -d' ' -f2-)
    # Make a folder for it.
    EXE_DIR="$OUTDIR/$EXE_FILE"
    mkdir -p $EXE_DIR

    OUTFILE="$EXE_DIR/make.out"
    
    # Save the output file path
    echo "$EXE_FILE" >> $OUTFILE

    # Resolve symlinked library paths.
    for i in ${!CMD_ARGS[@]}; do
        echo "${CMD_ARGS[$i]}" | grep -- "-L" > /dev/null
        if [ $? -eq 0 ]; then
            CMD_ARGS[$i]="-L$(realpath ${CMD_ARGS[$i]:2})"
        fi
    done
    
 
    # Write out the final output command.
    # If we had source files specified as arguments, rename them to objects.
    # We will build the relevant objects shortly.
    echo "$CMD ${CMD_ARGS[@]}" | sed -r "s@\.(c|cc|cpp)@\.o@g" >> $OUTFILE
      for i in ${CMD_ARGS[@]};
    do  
        # Find all object files and write them to make.out
        # Then, copy them to the working dir for this program.
        OBJ_FILE=$(echo "$i" | grep -c -E "\S*\.(o|a|lo|la)")
        if [ "$OBJ_FILE" == "1" ]; then
            echo -e "\t$i" >> $OUTFILE
            cp "$i" "$EXE_DIR/$(basename $i)" 
        fi
        # Find all source files, and build them into objects.
        # Redirect the output into the working dir.
        SRC_FILE=$(echo "$i" | grep -c -E "\S*\.(c|cc|cpp)")
        if [ "$SRC_FILE" == "1" ]; then
            NEW_OBJ="$(echo $(basename $i) | sed -r "s/\.(c|cc|ccp)/\.o/")"
            

            #orig
            #OBJ_CMD=$(echo "${CMD_ARGS[@]}" | sed -r "s/\S*\.(c|cc|cpp) //g" | sed "s@-o \S*@-o $EXE_DIR/$NEW_OBJ@g" | sed "s@-o@$i -o@g")
            
            #exploded
            STAGE_1=$(echo "${CMD_ARGS[@]}")
            STAGE_2_1=$(sed -r "s/\S*\.(c|cc|cpp) //g" <<< $STAGE_1)
            STAGE_2=$(sed -r 's/\S*\.(c|cc|cpp)$//g' <<< $STAGE_2_1)
            STAGE_3=$(sed "s@-o \S*@-o $EXE_DIR/$NEW_OBJ@g" <<< $STAGE_2)
            OBJ_CMD=$(sed "s@-o@$i -o@g" <<< $STAGE_3)
                       
            OBJ_CMD="-g -c $OBJ_CMD"
            $CMD $OBJ_CMD
            echo -e "\t$NEW_OBJ" >> $OUTFILE
        fi
    done   
fi

# Actually execute the command.
$CMD "${CMD_ARGS[@]}"
if [ "$?" != "0" ]; then
    echo "Compile command failed: $CMD ${CMD_ARGS[@]}"
    exit 1
fi

if [ "$IS_OBJ" = "0" ]; then
    # Capture the executable.  It's valuable on its own.
    cp $EXE_FILE $EXE_DIR/a.out
fi
