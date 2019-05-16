#!/bin/bash
#!/bin/sh

make warnings --silent

FILE=trabalho.o
if test -f "$FILE"; then
	for number in {1..100}
	do 
		./trabalho.o
	done 
fi
exit 0
