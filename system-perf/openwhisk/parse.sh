out=$1;
mkdir -p ${out}-logs;

mv /tmp/wsklogs/controller0/controller0_logs.log ${out}-logs;
scp zion05:/tmp/wsklogs/invoker0/invoker0_logs.log ${out}-logs;
scp zion06:/tmp/wsklogs/invoker1/invoker1_logs.log ${out}-logs;
scp zion07:/tmp/wsklogs/invoker2/invoker2_logs.log ${out}-logs;

ssh zion05 rm /tmp/wsklogs/invoker0/invoker0_logs.log;
ssh zion06 rm /tmp/wsklogs/invoker1/invoker1_logs.log;
ssh zion07 rm /tmp/wsklogs/invoker2/invoker2_logs.log;

screen -d -m -- nice -n 10 bash ./post-process.sh $out;
