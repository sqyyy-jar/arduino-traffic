setup:
	arduino-cli core install arduino:avr
	arduino-cli lib install "Adafruit SSD1306"

build:
	arduino-cli compile -b arduino:avr:mega traffic

run:
	arduino-cli compile -u -p /dev/ttyACM0 -b arduino:avr:mega traffic
	arduino-cli monitor -p /dev/ttyACM0

.PHONY: setup build run

.DEFAULT_GOAL := build
