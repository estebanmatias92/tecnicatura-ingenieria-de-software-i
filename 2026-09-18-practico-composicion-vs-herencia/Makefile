# Makefile — lab root runs from ./code/
.PHONY: all cpplayer test-car clean

all: cpplayer test-car

cpplayer:
	@echo "=== cpplayer (isft-poo-note) ==="
	cd isft-poo-note/examples/cpplayer-example/00-arecycle/01-cpp-player-init && bash build_and_execute.sh

test-car:
	@echo "=== cpp-test-evaluator (Car example) ==="
	cd cpp-test-evaluator/examples && bash build_test_car.sh

clean:
	@echo "=== clean ==="
	rm -f isft-poo-note/examples/cpplayer-example/00-arecycle/01-cpp-player-init/*.o \
	      isft-poo-note/examples/cpplayer-example/00-arecycle/01-cpp-player-init/*.bin
	rm -rf cpp-test-evaluator/examples/build
	rm -f cpp-test-evaluator/*.o
