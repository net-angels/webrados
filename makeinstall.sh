#!/bin/bash
:
rm -rf *.o
test -f /var/lib/apache2/fastcgi/radosweb.fcgi && rm /var/lib/apache2/fastcgi/radosweb.fcgi
gcc -Wall  -zmuldefs -g -ggdb -O0    -c -g -I/usr/include/rados -lconfig -o config.o config.c
gcc -Wall  -zmuldefs -g -ggdb -O0    -c -g -I/usr/include/rados -lconfig -o logs.o logs.c
gcc -Wall  -zmuldefs -g -ggdb -O0    -c -g -I/usr/include/rados -lconfig -o memory.o memory.c
gcc -Wall  -zmuldefs -g -ggdb -O0    -c -g -I/usr/include/rados -lconfig -o method.o method.c
gcc -Wall  -zmuldefs -g -ggdb -O0    -c -g -I/usr/include/rados -lconfig -o signal.o signal.c
gcc -Wall  -zmuldefs -g -ggdb -O0    -c -g -I/usr/include/rados -lconfig -o utceph.o utceph.c
mkdir -p /var/lib/apache2/fastcgi
gcc -Wall  -zmuldefs -g -ggdb -O0     -o /var/lib/apache2/fastcgi/radosweb.fcgi config.o logs.o memory.o method.o signal.o utceph.o /usr/lib/libfcgi.so /usr/lib/librados.so -lconfig -lpthread   
 
