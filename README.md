# u-boot dns320

u-boot support for dlink dns320 boards

tested with u-boot 2015.10

# how to use

clone u-boot git repository and copy content over.

```
wget ftp://ftp.denx.de/pub/u-boot/u-boot-2015.10.tar.bz2
tar xf u-boot-2015.10.tar.bz2
git clone https://github.com/avoidik/board_dns320.git
cp -r board_dns320/* u-boot-2015.10/
export CROSS_COMPILE=arm-linux-gnueabi-
make distclean && make dns320_config && make u-boot.kwb
./tools/kwboot -p -b u-boot.kwb -B115200 -t /dev/ttyUSB0
```

# credits

to maintainers and [Jamie Lentin](http://jamie.lentin.co.uk/devices/dlink-dns325/replacing-firmware/)
