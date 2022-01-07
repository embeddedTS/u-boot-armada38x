# u-boot-armada38x

This is the source for the U-boot used on the TS-7800-V2 and other TS boards based on Marvell's Armada 38x SoCs.<br>

Build it like so:<br>

export ARCH=arm<br>
export CROSS_COMPILE=&lt;path-to&gt;/arm-marvell-linux-gnueabi-<br> 
export CROSS_COMPILE_BH=&lt;path-to&gt;/arm-marvell-linux-gnueabi-<br>
./build.pl -b "armada_38x_ts7800v2" -m 3 -f mmc<br>


The toolchain may be downloaded from the following location:<br>

ftp://ftp.embeddedTS.com/ts-arm-sbc/ts-7800-v2-linux/cross-toolchains/armv7-marvell-linux-uclibcgnueabi-softfp_i686_64K_Dev_20131002.tar.bz2
