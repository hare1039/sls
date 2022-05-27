
name=$1;

d=$(date --iso-8601=seconds | sed 's/:/./g');

name="$d-$1";
echo "saving to $name"

mkdir $name
mv record.sh_* $name
mv $name ~/OneDrive/research/sls/system-perf/openwhisk
