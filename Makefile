TARGET = picamctl
CFLAGS = -Wall

ifeq (PI, 1)
	CFLAGS += -DPI
endif

SRC = src
OBJ = obj
BIN = bin
INSTALL_LOCATION = /usr/bin

SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

all: make_project_dirs target

target: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $(BIN)/$(TARGET)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

make_project_dirs:
	mkdir --parents $(OBJ) $(BIN)

install:
	cp $(BIN)/$(TARGET) $(INSTALL_LOCATION)/$(TARGET)

uninstall:
	rm $(INSTALL_LOCATION)/$(TARGET)

clean:
	rm -rf $(OBJ) $(BIN)

.PHONY: install uninstall clean make_project_dirs
