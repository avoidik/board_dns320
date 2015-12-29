# u-boot dns320

u-boot support for dlink dns320 boards and other own notes

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
```

# boot new u-boot

we need usb-ttl converter for this purposes

```
./tools/kwboot -p -b u-boot.kwb -B115200 -t /dev/ttyUSB0
```

we can copy u-boot.kwb file to ext2 usb flash drive and permanently overwrite u-boot

# overwrite u-boot

```
usb reset ; ext2load usb 0:1 0x1000000 /u-boot.kwb
nand erase 0x000000 0xe0000
nand write 0x1000000 0x000000 0xe0000
reset
```

# mtd-partitions layout

```
0x000000000000-0x000000100000 : "u-boot"
0x000000100000-0x000000600000 : "uImage" (<-- kernel)
0x000000600000-0x000000b00000 : "ramdisk" (<-- uRamdisk)
0x000000b00000-0x000007100000 : "image" (<-- image.cfs)
0x000007100000-0x000007b00000 : "mini firmware"
0x000007b00000-0x000008000000 : "config" (<-- default.tar.gz)
```

we can use dns323-firmware-tools to extract partitions from binary firmware and u-boot to write this partitions back from ext2 usb flash drive

```
setenv mtdparts 'mtdparts=orion_nand:1024k(u-boot),5120k(uImage),5120k(ramdisk),104448k(image),10240k(mini-firmware),5120k(config)'
usb reset

ext2load usb 0:1 0x100000 /mtd-orig/mtd1
nand erase.part uImage
nand write 0x100000 uImage

ext2load usb 0:1 0x100000 /mtd-orig/mtd2
nand erase.part ramdisk
nand write 0x100000 ramdisk

ext2load usb 0:1 0x100000 /mtd-orig/mtd3
nand erase.part image
nand write 0x100000 image

ext2load usb 0:1 0x100000 /mtd-orig/mtd4
nand erase.part mini-firmware
nand write 0x100000 mini-firmware

ext2load usb 0:1 0x100000 /mtd-orig/mtd5
nand erase.part config
nand write 0x100000 config
```

# recovery from tftp

use dns323-firmware-tools to extract partitions from binary firmware

```
setenv bootargs console=ttyS0,115200 root=/dev/ram0 init=/init tftpargs=192.168.1.100:192.168.1.1
setenv ipaddr 192.168.1.100
setenv serverip 192.168.1.1
tftp 0xa00000 uImage
tftp 0xf00000 uRamdisk; bootm 0xa00000 0xf00000
```

# compile kernel

- since 3.18 linux kernel already contains overlayfs code
- kirkwood configuration moved to mvebu_v5_defconfig

```
wget https://cdn.kernel.org/pub/linux/kernel/v3.x/linux-3.18.25.tar.xz
tar xf linux-3.18.25.tar.xz
cd linux-3.18.25
cp arch/arm/configs/mvebu_v5_defconfig .config
export ARCH=arm
export CROSS_COMPILE=arm-linux-gnueabi-
make distclean && make mrproper
make menuconfig
make zImage modules kirkwood-dns320.dtb
cat arch/arm/boot/dts/kirkwood-dns320.dtb >> arch/arm/boot/zImage
make uImage LOADADDR=0x8000
cp arch/arm/boot/uImage /mnt/boot/uImage
make INSTALL_MOD_PATH=/mnt/rootfs modules_install
sync
```

you can check my config in custom/.config

# late sata initialization

because of late init of the second sata port mdadm can't assemble array during startup, there is two ways to fix this.

## in userland 

1) open /etc/init.d/rcS
2) add sleep 30 before line

## in kernel (qnap patch)

drivers/ata/sata_mv.c

```
+//Patch by QNAP: delay SATA disk initialization
+#define	QMV_SATA_INIT_DELAY_PHASE	5000 //milliseconds
+////////////////////////////////////////////////////////////////
+
 /*
  * module options
  */
@@ -4329,7 +4333,11 @@
 		struct ata_port *ap = host->ports[port];
 		void __iomem *port_mmio = mv_port_base(hpriv->base, port);
 		unsigned int offset = port_mmio - hpriv->base;
-
+		// marvell 7042 port 2 port 3 will power on by order every  5 sec
+		if( (port==2) || (port == 3) ){
+			printk("Wait %d seconds to initialize scsi %d.\n",QMV_SATA_INIT_DELAY_PHASE/1000,port);
+			mdelay(QMV_SATA_INIT_DELAY_PHASE);
+		}
 		ata_port_pbar_desc(ap, MV_PRIMARY_BAR, -1, "mmio");
 		ata_port_pbar_desc(ap, MV_PRIMARY_BAR, offset, "port");
 	}
```

# credits

to maintainers and [Jamie Lentin](http://jamie.lentin.co.uk/devices/dlink-dns325/replacing-firmware/) and [Davide Del Grande](https://github.com/davidedg/NAS-DNS325-mod/)

dns320flash.c released under GPL by [greatlord](http://sourceforge.net/p/dns320/code/HEAD/tree/trunk/dns320_GPL/merge/dns320flash.c)
