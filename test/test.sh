#!/bin/bash
#
# This file is part of tcshParser.
# Copyright (C) 2013 Ellexus (www.ellexus.com)
#
# tcshParser is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

SCRIPT_PATH=`readlink -f $0`
TEST_PATH=`dirname $SCRIPT_PATH`

PROGRAM=$TEST_PATH/../src/tcshParser

ALIAS_FILE=$TEST_PATH/test-aliases.txt

# Run the program with a specific command line, and compare the output
# with what we expected, reporting either OK or ERROR.

check () {
    local L_INPUT=$1
    local L_EXPECT=$2
    local L_RESULT=$(mktemp)
    local L_DIFF_FILE=$(mktemp)

    $PROGRAM $ALIAS_FILE >$L_RESULT $L_INPUT 

    if echo $L_EXPECT | diff -b $L_RESULT - > $L_DIFF_FILE; then
        echo "OK: $L_INPUT"
    else
        echo "ERROR: $L_INPUT"
        cat $L_DIFF_FILE
    fi

    rm -f $L_RESULT $L_DIFF_FILE
}

check "a1" "du -l"
check "allargs one two three" "echo one two three"
check "allbutlastarg one two three" "echo one two"
check "onepipeanother" "one | another"
check "cdls /tmp" "cd /tmp && ls --color=tty"
check "commandandallbutlastarg alpha beta gamma" "echo commandandallbutlastarg alpha beta"
check "echospaces" "echo a b c"
check "firstarg first second third" "echo first"
check "firstthenlast first second third" "echo first ; echo third"
check "echoeverything one two three" "echo echoeverything one two three"
check "secondandthird one two three four" "echo two three"
check "secondarg one two three four" "echo two"
check "thisandthat first second" "echo first && echo second"
check "thisorthat first second" "echo first || echo second"
check "twice one two three" "echo one one"
check "twicenewline one " "echo one ; echo one"
check "wholecommand the cat sat on the mat" "echo wholecommand the cat sat on the mat"
check "wholecommand2 the cat sat on the mat" "echo wholecommand2 the cat sat on the mat"
check "outtopipe one" "first one | second"
check "alltopipe one" "first one |& second"
check "setcurrent" "set current=\`pwd\`"
check "appendtofile 1 2 3" "sh -c 'echo 1 2 3 >>/dev/null'"
check "subinbacktick 1 2 3" "first \`second 1 2 3\`"
check "twostar 1 2 3" "echo 2 3"
check "twostar 1" "echo"
check "twostaralt 1 2 3" "echo 2 3"
check "twolast 1 2 3 4 5" "echo 2 and 5"
check "twolast 1 2" "echo 2 and 2"
check "begintoend 1 2 3 4 5" "echo 1 2 3 4 5"
check "begintoendalt 1 2 3" "echo 1 2 3"
check "a 1 2 3 4 5 6" "echo this is b 1 && echo this is c 2"
check "a \"1 2 3\" \"4 5 6\"" "echo this is b 1 2 3 && echo this is c 4 5 6"
check "two 1 \"2 3 4\"" "echo 2 3 4"
