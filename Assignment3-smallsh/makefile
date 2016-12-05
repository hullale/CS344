all: build
	
clean:
	rm -f smallsh
	
build:
	dos2unix smallsh.c > /dev/null 2>&1
	gcc smallsh.c -o smallsh -std=c99