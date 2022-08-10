
folder=$1;
connection=$2;
repeat=$3;
dryrun=$4;

inited="false";
root_dir=".."

if [[ "$connection" == "" ]]; then
    echo "empty user count"
fi

rm -rf workspace;
cp -r "$folder" workspace;
mkdir -p output;
mkdir -p processed;

cd workspace;

init_once()
{
    if [[ "$inited" == "false" ]]; then
        npm install;
        serverless deploy;
    fi
    inited="true";
}

for i in {0..10000}; do
    testid=$(jq --raw-output ".tests[$i].testid" config.json);
    output=${root_dir}/output/result_${folder}_${testid}_${connection}.txt;
    processed=${root_dir}/processed/result_${folder}_${testid}_${connection}.txt;

    if [[ "$testid" == "null" ]]; then
        break;
    fi

    if [[ -f "$output" ]]; then
        echo "experiment $output exist. skip"
        python3 ${root_dir}/analyze.py $output | tee $processed;
        continue;
    fi

    if [[ "$dryrun" != "" ]]; then
        echo "would run $output";
        continue;
    fi

    init_once;

    echo invoker $(curl 172.17.0.1/invokers -L -k) > $output;

    echo "Running test $testid";
    jq --monochrome-output --compact-output ".tests[$i].payload" config.json | tr --delete '\n' > payload.json;

    cat ${root_dir}/templates/luabody-prefix.tpl \
        payload.json \
        ${root_dir}/templates/luabody-suffix.tpl \
        ${root_dir}/templates/works.lua > works.lua;

    action=$(jq --raw-output ".action" config.json);
    echo "action name: $action";

    for j in $(seq 1 $repeat); do
        wrk --script works.lua \
            --threads $(( $(nproc) > $connection ? $connection : $(nproc) )) \
            --connections $connection \
            --duration 50s \
            --latency \
            --timeout 1m \
            "https://172.17.0.1/api/v1/namespaces/_/actions/${action}?blocking=true" >> $output;
#            | tee --append $output;

    done
    python3 ${root_dir}/analyze.py $output | tee $processed;
    sleep 10;
done
