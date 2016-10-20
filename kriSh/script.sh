#!kriSh

gcc -o HelloWorld ./HelloWorld.c 
ls
./HelloWorld a1 a2 a3 a4 a5 <input.txt >output.txt 2>error.txt
ls
./HelloWorld b1 b2 b3 b4 b5 <input.txt >>output.txt 2>>error.txt
ls
./HelloWorld c1 c2 c3 c4 c5 <input.txt

#This is a random comment

cat input.txt
cat output.txt
cat error.txt
