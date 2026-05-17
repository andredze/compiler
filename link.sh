file_name="$1"

gcc elf/lib.o elf/${file_name}.o -o exec/${file_name} -lm