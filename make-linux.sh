#!/bin/bash

rm release/other/ft2-clone &> /dev/null

echo Compiling, please wait patiently...
gcc -D__LINUX_ALSA__ src/rtmidi/*.cpp src/gfxdata/*.c src/*.c -lSDL2 -lpthread -lasound -lstdc++ -lm -Wshadow -Winit-self -Wall -mno-ms-bitfields -Wno-missing-field-initializers -Wno-implicit-fallthrough -Wno-unused-result -Wno-strict-aliasing -Wextra -Wunused -Wunreachable-code -Wswitch-default -march=native -mtune=native -O3 -o release/other/ft2-clone

rm src/rtmidi/*.o src/gfxdata/*.o src/*.o &> /dev/null

echo Done! The executable is in the folder named \'release/other\'.
