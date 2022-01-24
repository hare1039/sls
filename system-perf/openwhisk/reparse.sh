
name=$1;
7z x $name && rm $name;

./post-process.sh ${name::-3};
