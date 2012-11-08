
prog = gcd kmp
all:$(prog)

gcd:gcd.c
	$(CC) -o $@ $<
kmp:kmp.c
	$(CC) -o $@ $<

clean:
	$(RM) $(prog)

.PHONY:clean
