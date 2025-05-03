#!/usr/bin/env zsh

#./runTests <executable> <directory with test files>

passed=0
failed=0
for f in "tests"/*.in
do
  basefile=$(basename -- "$f")
  #extension="${basefile##*.}"
  testfile="tests/${basefile%.*}.test"
  infile="tests/${basefile%.*}.in"
  outfile="tests/${basefile%.*}.out"
  difffile="tests/${basefile%.*}.diff"
  infofile="tests/${basefile%.*}.info"

  cat $infofile
  echo -n "..."
  start_time=$(gdate +%s%3N)
  ./dlx "$infile" < "$f" > $testfile
  end_time=$(gdate +%s%3N)
  elapsed_time=$((end_time - start_time)) # Convert nanoseconds to milliseconds if gdate is used
  if cmp -s "$testfile" "$outfile"
  then
    echo "passed in $elapsed_time ms"
    passed=$((passed+1))
    rm -f "$difffile"
  else
    echo "failed, see $difffile"
    diff "$testfile" "$outfile" > "$difffile"
    failed=$((failed+1))
  fi
  rm $testfile
done

echo $passed" PASSED out of "$(($failed+$passed))" ("$failed" failed)"

