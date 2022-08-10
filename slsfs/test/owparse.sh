out=$1;
mkdir -p ${out};

mv /tmp/wsklogs/controller0/controller0_logs.log ${out};
scp zion08:/tmp/wsklogs/invoker0/invoker0_logs.log ${out};
scp zion09:/tmp/wsklogs/invoker1/invoker1_logs.log ${out};
scp zion10:/tmp/wsklogs/invoker2/invoker2_logs.log ${out};

ssh zion08 rm /tmp/wsklogs/invoker0/invoker0_logs.log;
ssh zion09 rm /tmp/wsklogs/invoker1/invoker1_logs.log;
ssh zion10 rm /tmp/wsklogs/invoker2/invoker2_logs.log;

screen -d -m -- nice -n 10 bash ./post-process.sh $out;
