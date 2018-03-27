# Record Linking

Developing a C program to extract entries from census records to form
a list of potential links in preparation for ML training.

## Usage

To run the program, enter the ```src/``` directory.
Copy ```template``` to a file named ```config``` and edit this file as needed.
Finally, execute ```./run.sh```.
This shell script reads the configuration file, sorts the input data 
(if necessary), and executes the program.

You may want to redirect stdout and stderr to a file.
Using bash, you can do ```./run.sh &> file```.

## Requirements
* [GNU Make](https://www.gnu.org/software/make/)
* [gcc](https://gcc.gnu.org/)

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
