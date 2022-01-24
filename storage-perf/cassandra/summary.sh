filename=$1;

put_cassandra=$(cat $filename | grep 'put_cassandra' | awk '{ total += $2; count++ } END { printf "%.2f", total/count }');
get_cassandra=$(cat $filename | grep 'get_cassandra' | awk '{ total += $2; count++ } END { printf "%.2f", total/count }');

echo "put_cassandra $put_cassandra ns";
echo "get_cassandra $get_cassandra ns";
