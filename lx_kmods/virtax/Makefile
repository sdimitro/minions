obj-m += virtax.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install:
	sudo insmod virtax.ko

uninstall:
	sudo rmmod virtax

test: all install
	sudo dmesg | tail -5
	sleep 2
	$(MAKE) uninstall
	sudo dmesg | tail -5

check:
	@echo "Running kernel checkstyle and sparse analysis..."
	@echo "=== Running checkpatch.pl ==="
	@if [ -f /lib/modules/$(shell uname -r)/build/scripts/checkpatch.pl ]; then \
	        /lib/modules/$(shell uname -r)/build/scripts/checkpatch.pl --no-tree --file virtax.c; \
	else \
	        echo "checkpatch.pl not found. Trying alternative locations..."; \
	        if [ -f /usr/src/linux-headers-$(shell uname -r)/scripts/checkpatch.pl ]; then \
	                /usr/src/linux-headers-$(shell uname -r)/scripts/checkpatch.pl --no-tree --file virtax.c; \
	        elif [ -f /usr/src/linux/scripts/checkpatch.pl ]; then \
	                /usr/src/linux/scripts/checkpatch.pl --no-tree --file virtax.c; \
	        else \
	                echo "checkpatch.pl not found. Please install kernel headers or provide path manually."; \
	                echo "You can also run: sudo apt-get install linux-headers-\$$(uname -r)"; \
	        fi; \
	fi
	@echo ""
	@echo "=== Running sparse ==="
	@if command -v sparse >/dev/null 2>&1; then \
	        echo "Running sparse with kernel build system..."; \
	        make -C /lib/modules/$(shell uname -r)/build M=$(PWD) C=1; \
	else \
	        echo "sparse not found. Install with: sudo apt-get install sparse"; \
	        echo "Skipping sparse analysis..."; \
	fi

.PHONY: all clean install uninstall test check
