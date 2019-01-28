#
# Copyright (C) 2017 Mark Featherston <mark@embeddedarm.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifndef CONFIG_SPL_BUILD
obj-y	:= post.o ts7800-v2.o
else
obj-y	:= ts7800-v2.o
endif

obj-$(CONFIG_LED_STATUS) += led.o
