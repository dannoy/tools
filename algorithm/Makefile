
prog = gcd kmp partition_number
all:$(prog)

gcd:gcd.c
	$(CC) -o $@ $<
kmp:kmp.c
	$(CC) -o $@ $<
partition_number:partition_number.c
	$(CC) -o $@ $<

clean:
	$(RM) $(prog)

.PHONY:clean
