FQBN=arduino:avr:leonardo
PORT=COM3

build:
	arduino-cli compile -b $(FQBN)

flash:
	arduino-cli upload -b $(FQBN) -p $(PORT)

mon:
	arduino-cli monitor --config baudrate=57600 -p $(PORT)
