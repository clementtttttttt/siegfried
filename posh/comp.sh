
x86_64-siegfried-gcc -march=x86-64 -O0 -g -static-pie  main.c -o posh
cd ..
sudo losetup -Pf test.img
sudo mount /dev/loop0p1 mnt
sudo cp posh/posh mnt/sbin/sfinit
sudo umount mnt
sudo losetup -d /dev/loop0


