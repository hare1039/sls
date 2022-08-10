
out=$1;

python3 logparse.py $out/controller0_logs.log controller | tee -a $out/result-controller;
python3 logparse.py $out/invoker0_logs.log invoker | tee -a $out/result-invoker0;
python3 logparse.py $out/invoker1_logs.log invoker | tee -a $out/result-invoker1;
python3 logparse.py $out/invoker2_logs.log invoker | tee -a $out/result-invoker2;

#7z a -t7z -mx=9 -mfb=273 -ms -md=31 -myx=9 -mtm=- -mmt -mmtf -md=1536m -mmf=bt3 -mmc=70 -mpb=0 -mlc=0 -m0=LZMA2:27 -sdel $out.7z ${out}
