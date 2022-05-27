
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

build-string()
{
    local maxlen=$1;
    local v=""
    for i in $(seq 1 $maxlen); do
        v="a$v";
    done
    echo $v;
}

inited_function="false"
init-once()
{
    if [[ "$inited_function" == "false" ]]; then
        echo "init test $testid";
        local initsize=$(jq --monochrome-output \
                            --compact-output \
                            '.inits | length' config.json);
        for k in $(seq 0 $initsize); do
            local init=$(jq --monochrome-output \
                            --compact-output \
                            ".inits[$k].payload" config.json);

            if [[ "$(jq --monochrome-output --compact-output ".inits[$k].payload.operation" config.json)" != '"write"' ]]; then
                local init=$(jq --monochrome-output \
                                --compact-output \
                                ".inits[$k].payload" config.json);
            else
                local init=$(jq --monochrome-output \
                                --compact-output \
                                ".inits[$k].payload + { \"data\": \"$(build-string $datalen)\" }" config.json);
            fi

            if [[ "$init" != "null" ]]; then
                curl --insecure -X POST \
                     -H 'Authorization: Basic MjNiYzQ2YjEtNzFmNi00ZWQ1LThjNTQtODE2YWE0ZjhjNTAyOjEyM3pPM3haQ0xyTU42djJCS0sxZFhZRnBYbFBrY2NPRnFtMTJDZEFzTWdSVTRWck5aOWx5R1ZDR3VNREdJd1A=' \
                     -H "Content-Type: application/json" \
                     -H 'User-Agent: OpenWhisk-CLI/1.0 (2021-04-01T23:49:54.523+0000) linux amd64' \
                     -d "$init" \
                     https://172.17.0.1/api/v1/namespaces/_/actions/cpp?blocking=true 2>&1 1>/dev/null;
            fi
        done
    fi
}

for datalen in 1 16 64 256 1024 8096; do
    testsize=$(jq --monochrome-output \
                  --compact-output \
                  '.tests | length' config.json);
    for i in $(seq 0 $testsize); do
        testid=$(jq --raw-output ".tests[$i].testid" config.json);
        output=${root_dir}/output/result_${folder}_${testid}_${connection}_${datalen}.txt;
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

        echo invoker $(curl 172.17.0.1/invokers -L -k) > $output;
        init-once;

        echo "Running test $testid";
        if [[ "$(jq --monochrome-output --compact-output ".tests[$i].payload.operation" config.json)" != '"write"' ]]; then
            jq --monochrome-output \
               --compact-output \
               ".tests[$i].payload" config.json \
                | tr --delete '\n' > payload.json;
        else
            jq --monochrome-output \
               --compact-output \
               ".tests[$i].payload + { \"data\": \"$(build-string $datalen)\" }" config.json \
                | tr --delete '\n' > payload.json;
        fi

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
done
