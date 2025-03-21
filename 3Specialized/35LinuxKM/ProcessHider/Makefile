obj-m += ProcessHiderKM.o

PWD := $(CURDIR)
BUILD_DIR := $(PWD)/build

all:
	mkdir -p build
	make build/ProcessHiderKM.ko
	make build/ProcessHider.o
	make build/meter.o
	
build/ProcessHiderKM.ko: ProcessHiderKM.c
	mkdir -p build
	cd build
	cp $(PWD)/ProcessHiderKM.c $(BUILD_DIR)
	cp $(PWD)/Makefile $(BUILD_DIR)
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) modules
	rm -f $(BUILD_DIR)/ProcessHiderKM.c
	rm -f $(BUILD_DIR)/Makefile
	cd ..

build/ProcessHider.o: ProcessHider.cpp
	g++ -g -std=c++20 ProcessHider.cpp -o build/ProcessHider.o

build/meter.o: meter.cpp
	g++ -g -std=c++20 meter.cpp -o build/meter.o

clean:
	cd build
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(BUILD_DIR) clean
	rm -f ProcessHider.o
	rm -f meter.o
	cd ..
	