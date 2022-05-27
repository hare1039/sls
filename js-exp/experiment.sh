
skip=("templates"
      "workspace"
      "processed*"
      "archive"
      "output");

skip+=(
    "access-storage-direct"
    "access-storage-openwhisk-composer"
    "access-storage-call-openwhisk"
#    "computational-helloworld"
#    "computational-direct"
#    "computational-openwhisk-composer"
)

for exp in */ ; do
    exp=$(echo $exp | tr --delete '/');
    shouldskip="false";
    for s in ${skip[@]} ; do
        if [[ "$exp" == "$s" ]]; then
            shouldskip="true";
        fi
    done

    if [[ "$shouldskip" == "true" ]]; then
        continue;
    fi


    #                     connections repeat# dryrun
    tests=(
#        "./record.sh $exp 2            3"
        "./record.sh $exp 4            5"
        "./record.sh $exp 64           5"
        "./record.sh $exp 256          5"
        "./record.sh $exp 1024         5"
#        "./record.sh $exp 1           10"
#        "./record.sh $exp 2           10"
#        "./record.sh $exp 4           10"
#        "./record.sh $exp 8           10"
#        "./record.sh $exp 16          10"
#        "./record.sh $exp 32          10"
#        "./record.sh $exp 64          10"
#        "./record.sh $exp 128         10"
#        "./record.sh $exp 256         10"
#        "./record.sh $exp 512         10"
#        "./record.sh $exp 1024        3"
#        "./record.sh $exp 2048        10"
    );

    echo "dryrun check $exp";
    willrun="false";
    for i in "${tests[@]}"; do
        if $i --dryrun 2>/dev/null | grep -q 'would run'; then
            willrun="true";
            break;
        fi
    done

    if [[ "$willrun" == "true" ]]; then
        ./restart_openwhisk.sh
        for i in "${tests[@]}"; do
            $i;
            filename=$(echo $i | tr -s ' ' | sed 's/[ ]/_/g');
            bash -c "cd /home/hare1039/func/system-perf/openwhisk; ./parse.sh $filename"
        done
    fi
done
