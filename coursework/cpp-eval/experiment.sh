
skip=("templates"
      "workspace"
      "processed*"
      "archive"
      "output");

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
        "./record.sh $exp 1            5"
        "./record.sh $exp 4            5"
        "./record.sh $exp 64           5"
        "./record.sh $exp 256          5"
        "./record.sh $exp 1000         5"
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
        done
    fi
done
