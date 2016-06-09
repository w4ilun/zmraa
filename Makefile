ccflags-y +=-I$(srctree)/ext/lib/mraa/include
obj-$(CONFIG_MRAA) := source/mraa.o
obj-$(CONFIG_MRAA_GPIO) += source/gpio.o
obj-$(CONFIG_MRAA_I2C) += source/i2c.o
obj-$(CONFIG_MRAA_PWM) += source/pwm.o
obj-$(CONFIG_BOARD_ARDUINO_101) += source/arduino_101.o
obj-$(CONFIG_BOARD_ARDUINO_101_SSS) += source/arduino_101_sss.o
obj-$(CONFIG_BOARD_QUARK_D2000_CRB) += source/intel_d2k_crb.o
