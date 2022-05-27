
v=(1 8 32 64 128 512)
t=("usertable3" "usertable")

for i in "${v[@]}"; do

    if [[ "${i}" == "1" ]]; then
        cat <<EOF > ~/YCSB/large.dat
recordcount=50000
operationcount=50000
EOF
    else
        cat <<EOF > ~/YCSB/large.dat
recordcount=500000
operationcount=500000
EOF
    fi

    for table in "${t[@]}"; do
#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load cassandra-cql -threads $i -P workloads/workloada -P large.dat -p hosts=zion08,zion09,zion10 -p cassandra.keyspace=$table" -s | tee workloada_$table_$i.txt
#        echo "starting workload" >> workloada_$table_$i.txt
#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb run cassandra-cql -threads $i -P workloads/workloada -P large.dat -p hosts=zion08,zion09,zion10 -p cassandra.keyspace=$table" -s | tee -a workloada_$table_$i.txt
#
#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load cassandra-cql -threads $i -P workloads/workloadc -P large.dat -p hosts=zion08,zion09,zion10 -p cassandra.keyspace=$table" -s | tee workloadc_$table_$i.txt
#        echo "starting workload" >> workloadc_$table_$i.txt
#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb run cassandra-cql -threads $i -P workloads/workloadc -P large.dat -p hosts=zion08,zion09,zion10 -p cassandra.keyspace=$table" -s | tee -a workloadc_$table_$i.txt

        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load cassandra-cql -threads $i -P workloads/workloadf -P large.dat -p hosts=zion08,zion09,zion10 -p cassandra.keyspace=$table" -s | tee workloadf_$table_$i.txt
        echo "starting workload" >> workloadf_$table_$i.txt
        bash -c "cd ${HOME}/YCSB; ./bin/ycsb run cassandra-cql -threads $i -P workloads/workloadf -P large.dat -p hosts=zion08,zion09,zion10 -p cassandra.keyspace=$table" -s | tee -a workloadf_$table_$i.txt
    done
done
