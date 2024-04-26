# If you added a file to your project add it to the build section in the Makefile
# Always start the line with a tab in Makefile, it is its syntax

process_generator_deps = ./src/ds/queue.c ./src/utils.c
scheduler_deps = ./src/scheduling_algorithms.c ./src/ds/priority_queue.c ./src/utils.c

build:
	gcc ./src/process_generator.c ${process_generator_deps} -o ./bin/process_generator.out
	gcc ./src/scheduler.c ${scheduler_deps} -lm -o ./bin/scheduler.out
	gcc ./src/clk.c -o ./bin/clk.out
	gcc ./src/process.c -o ./bin/process.out
	gcc ./src/test_generator.c ./src/utils.c -o ./bin/test_generator.out

clean:
	rm -f ./bin/*.out  ./processes.txt

all: clean build

run:
	#./bin/test_generator.out
	./bin/process_generator.out
