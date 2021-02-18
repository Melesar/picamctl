TARGET = picamctl

$(TARGET): main.c
	$(CC) -Wall -o $@ $^

pi: main.c
	$(CC) -Wall -DPI -o $(TARGET) $^

install:
	cp $(TARGET) /usr/bin/$(TARGET)

clean:
	rm $(TARGET)

.PHONY: install clean
