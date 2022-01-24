
for i in *.7z; do
    screen -d -m -- nice -n 10 bash ./reparse.sh $i;
done
