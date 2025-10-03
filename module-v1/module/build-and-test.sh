killall -9 chuwi-minibook-x-tablet-mode

sudo rmmod chuwi-minibook-x-tablet-mode

make -C /lib/modules/$(uname -r)/build M=$(pwd) modules && \
    sudo insmod chuwi-minibook-x-tablet-mode.ko && \
    echo 0 | sudo tee /sys/kernel/chuwi-minibook-x-tablet-mode/force

