
PREFIX=/usr/local

ds4ctl: battery.c
	$(CC) -O $^ -o $@

install: ds4ctl
	mkdir -p $(PREFIX)/bin
	cp ds4ctl $(PREFIX)/bin

clean:
	rm -f ds4ctl
