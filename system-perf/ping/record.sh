ping -c 60 -q zion01 > zion01.txt &
ping -c 60 -q zion05 > zion05.txt &
ping -c 60 -q zion06 > zion06.txt &
ping -c 60 -q zion07 > zion07.txt &
ping -c 60 -q zion08 > zion08.txt &
ping -c 60 -q zion09 > zion09.txt &
ping -c 60 -q zion10 > zion10.txt &

echo recording for 60 seconds
wait < <(jobs -p);
