filename=$1;

put_etcd3=$(cat $filename | grep 'put_etcd3' | awk '{ total += $2; count++ } END { printf "%.2f", total/count }');
get_etcd3=$(cat $filename | grep 'get_etcd3' | awk '{ total += $2; count++ } END { printf "%.2f", total/count }');

echo "put_etcd3 $put_etcd3 ns";
echo "get_etcd3 $get_etcd3 ns";
