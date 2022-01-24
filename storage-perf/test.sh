
connection=$1;
testnum=$2;
if [[ "$testnum" == "" ]]; then
    echo "usage: $0 connection testnum"
    echo "  usage: $0 40 1000; # 40 connections and each connection get 1000 request and average";
    exit 1;
fi

for exp in */ ; do
    exp=$(echo $exp | tr --delete '/');

    echo parallel $connection;
    for i in $(seq 0 $connection); do
        bash -c "cd $exp; node run.js $testnum" > $exp-$connection-$i.txt &
    done
    wait < <(jobs -p);
    cat $exp-$connection-*.txt > $exp-$connection.txt;
    rm $exp-$connection-*.txt;
    $exp/summary.sh $exp-$connection.txt > $exp-$connection-processed.txt;
done
