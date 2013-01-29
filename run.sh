sudo chmod 777 /dev/ttyUSB0
sudo rm /dev/shm/achshm-ptu-c46*
ach -1 -C ptu-c46-ref -m 10 -n 3000
ach -1 -C ptu-c46-state -m 10 -n 3000
./ptu-c46-ach /dev/ttyUSB0 &
