#!/bin/bash

# set -x

get_test_name() {
    FILE=$1
    cat "$FILE" | grep "#!name" | sed 's/#!name //g'
}

get_test_expect() {
    FILE=$1
    cat "$FILE" | grep "#!expect" | sed 's/#!expect //g'
}

EXEC=$1
TESTDIR=$2
EXTENSION="nnl"

for test_file in $(find $TESTDIR -name "*.$EXTENSION"); do    
    test_name=$(get_test_name $test_file)
    if [[ "$test_name" == "" ]]; then
        continue
    fi

    echo "---"
    echo "File: $test_file"
    echo "Test: $test_name"

    EXPECT=$(get_test_expect $test_file)
    ACTUAL=$($EXEC $test_file 2>&1)

    if [[ "$EXPECT" == "$ACTUAL" ]]; then
        echo "Status: OK"
    else
        echo "Status: FAIL"
        echo "See difference between expected and actual output":
        echo "$EXPECT" >/tmp/nnl_expect
        echo "$ACTUAL" >/tmp/nnl_actual
        diff --side-by-side /tmp/nnl_{expect,actual}
        break
    fi
done