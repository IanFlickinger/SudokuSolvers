build:
	cmake -B build -G "Unix Makefiles"

clean:
	rm -rf build

rebuild: 
	make clean
	make build