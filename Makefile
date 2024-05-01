# If you added a file to your project add it to the build section in the Makefile
# Always start the line with a tab in Makefile, it is its syntax

process_generator_deps = ./src/ds/queue.c ./src/utils.c
scheduler_deps = ./src/scheduling_algorithms.c ./src/ds/queue.c ./src/ds/fib_heap.c ./src/utils.c ./src/gui/task_manager.c ./src/gui/page_init.c

build:
	cc ./src/gui/gui.c ./src/utils.c -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o ./bin/synergify.out
	gcc ./src/process_generator.c ${process_generator_deps} -o ./bin/process_generator.out
	cc ./src/scheduler.c ${scheduler_deps} -pthread -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o ./bin/scheduler.out
	gcc ./src/clk.c -o ./bin/clk.out
	gcc ./src/process.c -o ./bin/process.out
	gcc ./src/test_generator.c ./src/utils.c -o ./bin/test_generator.out

clean:
	rm -f ./bin/*.out  ./processes.txt

all: clean build

run:
	#./bin/test_generator.out
	./bin/synergify.out
