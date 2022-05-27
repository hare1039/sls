
single()
{
    local REMOTE_HOST="$1";
    local length="$2";

    iperf3 -c ${REMOTE_HOST} -l ${length} --time 30           --logfile ${REMOTE_HOST}_${length}_ctos.txt
    iperf3 -c ${REMOTE_HOST} -l ${length} --time 30 --reverse --logfile ${REMOTE_HOST}_${length}_stoc.txt
    iperf3 -c ${REMOTE_HOST} -l ${length} --time 30 --bidir   --logfile ${REMOTE_HOST}_${length}_bidir.txt

    rm -f ${REMOTE_HOST}_${length}.txt
    echo "hare >> ${REMOTE_HOST}" | tee -a ${REMOTE_HOST}_${length}.txt
    cat ${REMOTE_HOST}_${length}_ctos.txt >> ${REMOTE_HOST}_${length}.txt

    echo "hare << ${REMOTE_HOST}" | tee -a ${REMOTE_HOST}_${length}.txt;
    cat ${REMOTE_HOST}_${length}_stoc.txt >> ${REMOTE_HOST}_${length}.txt;

    echo "hare <=> ${REMOTE_HOST}" | tee -a ${REMOTE_HOST}_${length}.txt
    cat ${REMOTE_HOST}_${length}_bidir.txt >> ${REMOTE_HOST}_${length}.txt

    rm ${REMOTE_HOST}_${length}_*.txt
}

for i in 1 8 64 256 512 1K 4K 8K 16K; do
    single zion01 $i;
#    single zion07 $i;
#    single zion08 $i;
#    single zion09 $i;
#    single zion10 $i;
done
