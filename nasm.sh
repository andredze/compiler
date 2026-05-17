file_name="$1"

nasm -f elf64 -g -F dwarf asm/${file_name}.asm -o elf/${file_name}.o
./link.sh ${file_name}