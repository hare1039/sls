
exp=$1;
connection=$2;
testnum=$3;
if [[ "$testnum" == "" ]]; then
    echo "usage: $0 folder connection testnum"
    echo "  usage: $0 etcd3 40 1000; # 40 connections and each connection get 1000 request and average";
    exit 1;
fi

echo parallel $connection;
for i in $(seq 0 $connection); do
    bash -c "cd $exp; node run.js $testnum" > $exp-$connection-$i.txt &
done
wait < <(jobs -p);
cat $exp-$connection-*.txt > $exp-$connection.txt;
rm $exp-$connection-*.txt;
$exp/summary.sh $exp-$connection.txt > $exp-$connection-processed.txt;
