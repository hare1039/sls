OW=/home/hare1039/openwhisk;
export ENVIRONMENT=zion
cd $OW/ansible;
sudo docker rm -f -v couchdb;

ssh zion05 'bash -c "docker ps; cd /home/hare1039/workspace/become-root; ./restart-docker.sh"' &
ssh zion06 'bash -c "docker ps; cd /home/hare1039/workspace/become-root; ./restart-docker.sh"' &
ssh zion07 'bash -c "docker ps; cd /home/hare1039/workspace/become-root; ./restart-docker.sh"' &
wait < <(jobs -p);

sudo ansible-playbook -i environments/$ENVIRONMENT -u hare1039 couchdb.yml
sudo ansible-playbook -i environments/$ENVIRONMENT -u hare1039 initdb.yml
sudo ansible-playbook -i environments/$ENVIRONMENT -u hare1039 wipe.yml
sudo ansible-playbook -i environments/$ENVIRONMENT -u hare1039 openwhisk.yml
