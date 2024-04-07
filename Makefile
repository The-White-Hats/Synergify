build:
	gcc ./src/process_generator.c -o ./bin/process_generator.out
	gcc ./src/clk.c -o ./bin/clk.out
	gcc ./src/scheduler.c -o ./bin/scheduler.out
	gcc ./src/process.c -o ./bin/process.out
	gcc ./src/test_generator.c -o ./bin/test_generator.out

clean:
	rm -f ./bin/*.out  ./bin/processes.txt

all: clean build

run:
	./bin/process_generator.out
