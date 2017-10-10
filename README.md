# Record Linking

Developing a C program to extract entries from census records to form
a list of potential links in preparation for ML training.

## Usage

To run the program, edit ```src/config``` as needed and then execute
```cd src/ && ./run.sh```.  This shell script reads the configuration file,
sorts the input data (if necessary), and executes the program.

## Mac OS
OpenMP support is required for parallel performance. 
To use OpenMP, please make sure you have gcc installed on your machine and not
a symlink to Clang. You can check this with ```gcc --version```.

To install gcc, you will need [Homebrew](https://brew.sh/).
Install gcc using ```brew install gcc --without-multilib```. 
This should install gcc in ```/usr/local/bin``` as ```gcc-X```, where ```X```
is the version number.
Add ```/usr/local/bin``` to your path if it is not there already.
Finally, in ```src/Makefile```, change the first line to read ```CC=gcc-X```.


## References
* Feigenbaum, James J. "Automated census record linking: A machine learning approach." (2016).
