
all: sqlite-bench

gen-sql: gen-sql.py
	python3 $<

sqlite-bench: sqlite-bench.c
	gcc -O3 -lsqlite3 -o $@ $<

clean:
	rm -rf *~ *.o *.sql

mrproper: clean
	rm -rf sqlite-bench

.PHOMY: all clean mrproper gen-sql
