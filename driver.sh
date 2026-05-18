file_name="$1"

echo -n "Фронтенд: нажмите любую клавишу: [s - пропустить] "

read -s -n 1 key_press

if [ "$key_press" != "s" ]; then
    ./front psy/${file_name}.psy
fi

printf "\n"

echo -n "Бэкенд: нажмите любую клавишу: [s - пропустить] "

read -s -n 1 key_press

if [ "$key_press" != "s" ]; then
    ./back ast/${file_name}.txt
fi

printf "\n"

printf "Компиляция библиотеки: нажмите любую клавишу: [s - пропустить] "

read -s -n 1 key_press

if [ "$key_press" != "s" ]; then
    gcc -ggdb3 -static -c lib.c -o elf/lib.o
fi

printf "\n"

echo -n "Линковка: нажмите любую клавишу: [s - пропустить] "

read -s -n 1 key_press

if [ "$key_press" != "s" ]; then
    ./link.sh ${file_name}
fi

printf "\n"