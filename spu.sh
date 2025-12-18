filename="$1"
./spu-asm/compile asm/${filename}.asm bin/${filename}.bin LIST=1
./spu-asm/run bin/${filename}.bin
