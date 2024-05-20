trap '' SIGINT 
 nohup make test & disown 
gdb -ex 'target remote localhost:1234' sfkrnl.elf


killall qemu-system-x86_64 -9

trap SIGINT
