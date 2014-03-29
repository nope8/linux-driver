insmod ./globalfifo_async.ko 
mknod  /dev/globalfifo c 250 0
chmod  777  /dev/globalfifo
#./a.out
