cc -O2 -o chuwi-minibook-x-tablet-mode chuwi-minibook-x-tablet-mode.c -lm && \
    sudo cp chuwi-minibook-x-tablet-mode /usr/local/sbin/ && \
    sudo chuwi-minibook-x-tablet-mode iio:device0 iio:device1 100
