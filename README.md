# FDIROpener<br>

FDIROpener is a free and open-source portable command-line tool used to pack and unpack files.<br>
FDIROpener creates and opens .FDIR files, a very minimal and basic file archive format.<br>

# Instructions for Use

# Building from source

Step 1. Download the source as a ZIP and unzip.<br>><br>

Step 2. If using the Makefile, run the `make` command. The Makefile is already provided for you. (Note: You will need `gcc` and `x86_64-w64-mingw32-g++` to build using the Makefile)<br><br>

Step 2. If not using the Makefile, simply build the C++ executable using `g++` or `clang++`. If building for Windows systems, make sure to include the compiler flags `--static -static-libgcc -static-libstdc++` for the executable to work.

Step 3. Run using the below arguments.


# Portable binary

Step 1. Go to the Releases page and download the latest executable for your system.<br><br>

Step 2. Open your terminal or command line and navigate to the directory where the executable is.<br><br>

Step 3. Run the executable. Depending on what you want to do, you will have to use different arguments.<br><br>

If you want to write files to a .FDIR then use this command:<br>
`-i <input file> OR <input folder> -o <fdir name>`<br><br>

If you want to extract files from a .FDIR file then use this command:<br>
`-e <.fdir file> -o <output directory>`<br><br>


# Supported Operating Systems:<br>

Windows x86_64<br>
Linux x64<br>
