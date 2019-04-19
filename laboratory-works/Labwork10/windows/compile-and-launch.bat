rmdir build /s
mkdir build
gcc src/main.c -o build/main.exe
gcc src/child.c -o build/child.exe
cd build
main.exe
cd ..
