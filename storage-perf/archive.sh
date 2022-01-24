
if [[ "$1" == "" ]]; then
    echo archive.sh experiment_name;
    exit 0;
fi

shopt -s extglob;

d=$(date --iso-8601=seconds | sed 's/:/./g');

name="$d-$1";
echo "saving to $name"

mkdir -p "${HOME}/OneDrive/research/sls/storage-perf/archive/$name";
mv *.txt "${HOME}/OneDrive/research/sls/storage-perf/archive/$name";

cd ${HOME}/OneDrive/research/sls/storage-perf/archive;
7z a -t7z -mx=9 -mfb=273 -ms -md=31 -myx=9 -mtm=- -mmt -mmtf -md=1536m -mmf=bt3 -mmc=70 -mpb=0 -mlc=0 -m0=LZMA2:27 -mdel\
   $name.7z $name;
