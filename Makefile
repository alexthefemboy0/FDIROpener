C = g++
WIN_C = x86_64-w64-mingw32-g++
WINFLAGS = --static -static-libgcc -static-libstdc++
SRC_DIR = src
BIN_DIR = bin

.PHONY = clean
all: FDIROpenerLinux FDIROpenerWindows

# Build for Linux

FDIROpenerLinux: $(BIN_DIR)/FDIROpenerLinux
$(BIN_DIR)/FDIROpenerLinux: $(SRC_DIR)/main.cpp
	$(C) $(SRC_DIR)/main.cpp -o $(BIN_DIR)/FDIROpenerLinux

# Build for Windows

FDIROpenerWindows: $(BIN_DIR)/FDIROpenerWindows.exe
$(BIN_DIR)/FDIROpenerWindows.exe: $(SRC_DIR)/main.cpp
		$(WIN_C) $(SRC_DIR)/main.cpp -o $(BIN_DIR)/FDIROpenerWindows.exe $(WINFLAGS)

clean:
	rm -f $(BIN_DIR)/*