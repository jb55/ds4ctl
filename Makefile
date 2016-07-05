
ds4ctl: battery.c
	$(CC) -O $^ -o $@

clean:
	rm -f ds4ctl
