all: ld_poison.so mac

mac: mac.c
	gcc -Wall mac.c -o mac

ld_poison.so: ld_poison.c
	gcc -Wall -fPIC -shared -ldl ld_poison.c -o ld_poison.so

clean:
	rm ld_poison.so mac

