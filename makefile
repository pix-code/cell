all: watch

watch:
	while true; do \
		inotifywait -qr -e modify -e create -e delete -e move src/main.c; \
		clear; \
		make -s run; \
	done

run: bin/cell
	./$< $(ARGS)

bin/cell: cell.o
	ld $^ -o $@

cell.o: cell.asm
	as $^ -o $@

cell.asm: bin/main
	$^

bin/main: src/main.c
	gcc -O3 -Wall $^ -o $@

clean: tidy
	rm bin/main

tidy:
	rm cell.asm cell.o

.PHONY: build clean run watch tidy
