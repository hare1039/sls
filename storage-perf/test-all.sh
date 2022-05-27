
connection=$1;
testnum=$2;
if [[ "$testnum" == "" ]]; then
    echo "usage: $0 connection testnum"
    echo "  usage: $0 40 1000; # 40 connections and each connection get 1000 request and average";
    exit 1;
fi


for exp in */ ; do
    exp=$(echo $exp | tr --delete '/');
    ./test.sh $exp $connection $testnum;
done
