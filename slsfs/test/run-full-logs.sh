
execname="$1";

if [[ "$execname" == "" ]]; then
    echo "$execname empty. please supply which py script."
    exit -1;
fi

ssh zion01 screen -S caddylogger -dm "bash -c 'cd /home/hare1039/workspace/logdebugger; ./caddy run | tee /home/hare1039/workspace/logdebugger/v3-log.log'"

./restart-ow.sh

bash -c 'cd ../datafunction/; BUILDONLY=TRUE make function' &
bash -c 'cd ../app/; BUILDONLY=TRUE make function' &

wait;
mkdir -p templog;

python3 $execname | tee templog/runlog.log;


scp zion01:/home/hare1039/workspace/logdebugger/v3-log.log templog

ssh zion01 /home/hare1039/workspace/logdebugger/killcaddy.sh
./owparse.sh templog
