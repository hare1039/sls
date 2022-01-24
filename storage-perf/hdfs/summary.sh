filename=$1;

put_hdfs_total=$(cat $filename | grep 'put_hdfs' | awk '{ total += $2; count++ } END { printf "%.2f", total/count }')
put_hdfs_namenode=$(cat $filename | grep 'put_hdfs' | awk '{ total += $3; count++ } END { printf "%.2f", total/count }')
put_hdfs_datanode=$(cat $filename | grep 'put_hdfs' | awk '{ total += $4; count++ } END { printf "%.2f", total/count }')

get_hdfs_total=$(cat $filename | grep 'get_hdfs' | awk '{ total += $2; count++ } END { printf "%.2f", total/count }')
get_hdfs_namenode=$(cat $filename | grep 'get_hdfs' | awk '{ total += $3; count++ } END { printf "%.2f", total/count }')
get_hdfs_datanode=$(cat $filename | grep 'get_hdfs' | awk '{ total += $4; count++ } END { printf "%.2f", total/count }')

echo "put_hdfs total $put_hdfs_total ns, namenode $put_hdfs_namenode ns, datanode $put_hdfs_datanode ns";
echo "get_hdfs total $get_hdfs_total ns, namenode $get_hdfs_namenode ns, datanode $get_hdfs_datanode ns";
