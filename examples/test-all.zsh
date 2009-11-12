#!/usr/bin/env zsh

for (( x=0; $x < 29; $((x++)) ))
do
	if [ $x -lt 10 ]; then; y="0$x"; else; y="$x"; fi
	echo "1-$x"
	php test.php $x > result1-$y.txt 2>&1
done

echo "1-99"
php test.php 99 > result1-99.txt 2>&1


for (( x=0; $x < 29; $((x++)) ))
do
	if [ $x -lt 10 ]; then; y="0$x"; else; y="$x"; fi
	echo "2-$x"
	php test2.php $x > result2-$y.txt 2>&1
done

echo "2-99-0"
php test2.php 99 0 > result2-99-0.txt 2>&1
echo "2-99-1"
php test2.php 99 1 > result2-99-1.txt 2>&1
echo "2-99-2"
php test2.php 99 2 > result2-99-2.txt 2>&1
echo "2-99-3"
php test2.php 99 3 > result2-99-3.txt 2>&1
echo "2-99-4"
php test2.php 99 4 > result2-99-4.txt 2>&1


for (( x=0; $x < 29; $((x++)) ))
do
	if [ $x -lt 10 ]; then; y="0$x"; else; y="$x"; fi
	echo "3-$x"
	php test3.php $x > result3-$y.txt 2>&1
done

echo "3-99"
php test3.php 99 > result3-99.txt 2>&1


for (( x=0; $x < 29; $((x++)) ))
do
	if [ $x -lt 10 ]; then; y="0$x"; else; y="$x"; fi
	echo "4-$x"
	php test4.php $x > result4-$y.txt 2>&1
done

echo "4-99"
php test4.php 99 > result4-99.txt 2>&1


cat result*.txt > all-resulsts.txt
