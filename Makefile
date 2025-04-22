BAUD 	?= 115200
FQBN 	?= rak_rui:stm32:WisDuoRAK3172EvaluationBoard
DEBUG 	?= 1
PORT 	?= /dev/ttyUSB0

.PHONY: build

build:
	arduino-cli compile --build-path=build --fqbn="${FQBN}" --build-property="build.extra_flags=-DDEBUG=${DEBUG}"

flash:
	arduino-cli upload -p "${PORT}" --fqbn="${FQBN}" --build-path=build

monitor:
	arduino-cli monitor -p "${PORT}" --config="${BAUD}"
