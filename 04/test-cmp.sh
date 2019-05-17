#!/bin/sh
key=$1
cd `dirname $0`
ruby pty.rb './mysh2 -q' <t/${key} 2>${key}-err.txt | tee ${key}-out.txt
echo

# fire emoji to FIRE
perl -i -pe 's/\xF0\x9F\x94\xA5/FIRE/g' ${key}-out.txt

if diff -ub t/ans/${key}-ans.txt ${key}-out.txt; then
    echo "✅   OK ${key} 🎉"
else
    echo "❌   NG ${key} 😱"
fi
