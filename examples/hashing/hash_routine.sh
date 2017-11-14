BIN_HASH=`md5sum bin/*.bin | cut -d ' ' -f 1`
echo "hash of compiled source:" $BIN_HASH
mkdir -p .diffs
LATEST_GIT_COMMIT=`git log | head -1 | cut -d ' ' -f 2`
echo "latest git commit:" $LATEST_GIT_COMMIT
echo "DIFF BASED OFF COMMIT" $LATEST_GIT_COMMIT > .diffs/${BIN_HASH}.txt
echo "" >> .diffs/${BIN_HASH}.txt
git diff >> .diffs/${BIN_HASH}.txt
# feed in hash as bin flag
