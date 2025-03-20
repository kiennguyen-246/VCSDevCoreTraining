obj-m += ProcessHiderKM.o

PWD := $(CURDIR)

all:
	make ProcessHiderKM.ko
	make ProcessHider.o
	make meter.o
	
ProcessHiderKM.ko: ProcessHiderKM.c
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

ProcessHider.o: ProcessHider.cpp
	g++ -g -std=c++20 ProcessHider.cpp -o ProcessHider.o

meter.o: meter.cpp
	g++ -g -std=c++20 meter.cpp -o meter.o

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	