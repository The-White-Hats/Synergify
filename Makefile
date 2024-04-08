# If you added a file to your project add it to the build section in the Makefile
# Always start the line with a tab in Makefile, it is its syntax

build:
	gcc ./src/process_generator.c -o ./bin/process_generator.out
	gcc ./src/clk.c -o ./bin/clk.out
	gcc ./src/scheduler.c -o ./bin/scheduler.out
	gcc ./src/process.c -o ./bin/process.out
	gcc ./src/test_generator.c -o ./bin/test_generator.out

clean:
	rm -f ./bin/*.out  ./processes.txt

all: clean build

run:
	./bin/test_generator.out
	./bin/process_generator.out
