BOARD_TAG = atmega328-20
ARDUINO_PORT = /dev/cu.usb*

ARDUINO_SKETCHBOOK = .
ARDUINO_DIR = /Applications/Arduino.app/Contents/Resources/Java
ARDMK_DIR = ./libraries/Makefile
MONITOR_BAUDRATE = 19200

ISP_PROG = -c stk500v1
ISP_PORT = $(ARDUINO_PORT)
AVRDUDE_ARD_BAUDRATE = $(MONITOR_BAUDRATE)
AVRDUDE_CONF = $(AVR_TOOLS_DIR)/etc/avrdude.conf -b $(AVRDUDE_ARD_BAUDRATE)

ARDUINO_LIBS = SD SD/utility Buttons SoftwareSerial

include ./libraries/Makefile/arduino-mk/Arduino.mk
