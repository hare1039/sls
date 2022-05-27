

v=(1 8 32 64 256)
t=("usertable")

for i in "${v[@]}"; do

    if [[ "${i}" == "1" ]]; then
        cat <<EOF > ~/YCSB/large.dat
recordcount=100000
operationcount=100000
EOF
    else
        cat <<EOF > ~/YCSB/large.dat
recordcount=500000
operationcount=500000
EOF
    fi

    for table in "${t[@]}"; do

#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloada -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee workloada_$table_$i.txt
#        echo "starting workload" >> workloadf_$table_$i.txt
#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloada -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee -a workloada_$table_$i.txt

#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloadc -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee workloadc_$table_$i.txt
#        echo "starting workload" >> workloadf_$table_$i.txt
#        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloadc -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee -a workloadc_$table_$i.txt

        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloada -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee workloada_$table_$i.txt
        echo '---' >> workloada_$table_$i.txt
        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloada -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee -a workloada_$table_$i.txt

        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloadc -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee workloadc_$table_$i.txt
        echo '---' >> workloadc_$table_$i.txt
        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloadc -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee -a workloadc_$table_$i.txt

        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloadf -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee workloadf_$table_$i.txt
        echo '---' >> workloadf_$table_$i.txt
        bash -c "cd ${HOME}/YCSB; ./bin/ycsb load hbase2 -threads $i -P workloads/workloadf -P large.dat -cp ~/hbase/conf -p table=usertable -p columnfamily=family -s"  | tee -a workloadf_$table_$i.txt

    done
done
