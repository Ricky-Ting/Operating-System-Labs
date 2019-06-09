mv main.c main.cc
make
sudo cp libkvdb-32.so /usr/lib/
mv main.cc main.c
gcc  -m32 -o main main.c -L. -lkvdb-32
./main
