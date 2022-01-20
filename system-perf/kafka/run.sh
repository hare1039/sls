
OUTPUT=$PWD/result.txt;

cd ~/kafka;

bin/kafka-producer-perf-test.sh --topic invoker0 --throughput -1 --num-records 3000000 --record-size 1024 --producer-props acks=all bootstrap.servers=172.17.0.5:9093 | tee $OUTPUT
bin/kafka-producer-perf-test.sh --topic invoker1 --throughput -1 --num-records 3000000 --record-size 1024 --producer-props acks=all bootstrap.servers=172.17.0.5:9093 | tee -a $OUTPUT
