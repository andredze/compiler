filename="$1"
./spu-asm/compile asm/${filename}.asm bin/${filename}.bin
./spu-asm/run bin/${filename}.bin
