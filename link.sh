file_name="$1"

gcc elf/lib.o elf/$1.o -o exec/$1 -lm