OW=/home/hare1039/openwhisk;
export ENVIRONMENT=zion
cd $OW/ansible;
USE_SUDO="";

$USE_SUDO docker rm -f -v couchdb;

#ssh zion05 'bash -c "docker ps; cd /home/hare1039/workspace/become-root; ./restart-docker.sh"' &
#ssh zion06 'bash -c "docker ps; cd /home/hare1039/workspace/become-root; ./restart-docker.sh"' &
#ssh zion07 'bash -c "docker ps; cd /home/hare1039/workspace/become-root; ./restart-docker.sh"' &
#wait < <(jobs -p);

$USE_SUDO ansible-playbook -i environments/$ENVIRONMENT -u hare1039 setup.yml
$USE_SUDO ansible-playbook -i environments/$ENVIRONMENT -u hare1039 openwhisk.yml -e mode=clean
$USE_SUDO ansible-playbook -i environments/$ENVIRONMENT -u hare1039 invoker.yml -e mode=clean

ssh zion08 'bash -c "docker rm -f invoker0; rm /tmp/wsklogs/invoker0/invoker0_logs.log"' &
ssh zion09 'bash -c "docker rm -f invoker1; rm /tmp/wsklogs/invoker1/invoker1_logs.log"' &
ssh zion10 'bash -c "docker rm -f invoker2; rm /tmp/wsklogs/invoker2/invoker2_logs.log"' &

wait

$USE_SUDO ansible-playbook -i environments/$ENVIRONMENT -u hare1039 couchdb.yml
$USE_SUDO ansible-playbook -i environments/$ENVIRONMENT -u hare1039 initdb.yml
$USE_SUDO ansible-playbook -i environments/$ENVIRONMENT -u hare1039 wipe.yml

$USE_SUDO ansible-playbook -i environments/$ENVIRONMENT -u hare1039 openwhisk.yml
