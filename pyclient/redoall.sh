bash -c "cd ${HOME}/openwhisk; ./gradlew distDocker"
bash -c "cd ${HOME}/openwhisk; ./transfer_images.sh"
bash -c "cd ${HOME}/func/slsfs/test; ./restart-ow.sh"

bash -c "cd ${HOME}/func/js-exp/workspace; npm install; sls deploy" &
bash -c "cd ${HOME}/func/slsfs/datafunction; time make function" &
wait;

python3 client.py
