bin=sqlite-bench.$(shell uname -m)

all: $(bin)

gen-sql: gen-sql.py
	python3 $<

$(bin): sqlite-bench.c
	$(CC) -O3 -lsqlite3 -o $@ $<

clean:
	rm -rf *~ *.o *.sql

mrproper: clean
	rm -rf $(bin)

.PHOMY: all clean mrproper gen-sql
