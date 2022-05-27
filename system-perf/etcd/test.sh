

etcd-test-put()
{
    conn=$1;
    client=$2;
    args=$3;
    suffix=$4;

    out="resultput_$action_conn${conn}_client${client}_${suffix}.txt";

    benchmark --endpoints=zion08:23792,zion09:23792,zion10:23792 \
              $args --conns=$conn --clients=$client --precise \
              put --key-size=8 --sequential-keys --total=100000 --val-size=256 \
        2>&1 | tee $out;

    echo 'zion08:23792/metrics' >> $out;
    curl zion08:23792/metrics >> $out;

    echo 'zion09:23792/metrics' >> $out;
    curl zion09:23792/metrics >> $out;

    echo 'zion10:23792/metrics' >> $out;
    curl zion10:23792/metrics >> $out;
}

etcd-test-get()
{
    conn=$1;
    client=$2;
    args=$3;
    suffix=$4;

    out="resultget_$action_conn${conn}_client${client}_${suffix}.txt";

    benchmark --endpoints=zion08:23792,zion09:23792,zion10:23792 \
              $args --conns=$conn --clients=$client --precise \
              range stepkey --total=100000 \
        2>&1 | tee $out;

    echo 'zion08:23792/metrics' >> $out;
    curl zion08:23792/metrics >> $out;

    echo 'zion09:23792/metrics' >> $out;
    curl zion09:23792/metrics >> $out;

    echo 'zion10:23792/metrics' >> $out;
    curl zion10:23792/metrics >> $out;
}

etcd-test-put   1    1 "--target-leader" "target-leader";
etcd-test-put 100 1000 "--target-leader" "target-leader";
etcd-test-put   1    1 ;
etcd-test-put 100 1000 ;

etcd-test-get   1    1 "--consistency=l" "consistency-linearizable";
etcd-test-get 100 1000 "--consistency=l" "consistency-linearizable";

etcd-test-get   1    1 "--consistency=s" "consistency-serializable";
etcd-test-get 100 1000 "--consistency=s" "consistency-serializable";
