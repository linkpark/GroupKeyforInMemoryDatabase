### Group key index implementantions --- an column key index system

based on dictionary compress, and bit compress.

## Prerequisites 
Compile this project need buildessential, g++, cmake, glog, gtest, and boost.

1. Install buildessential, g++, cmake and boost
```bash
    sudo apt-get install build-essential g++
	sudo apt-get install cmake
	sudo apt-get install libboost-all-dev
```

2. Install glog:
```bash
	git clone https://github.com/google/glog
	cd glog
	mkdir build 
	cd build
	cmake ..
	make -j2
	sudo make install
```

3. Install google test:
```bash 
	git clone https://github.com/google/googletest
	cd googletest
	mkdir build
	cd build
	cmake ..
	make -j2
	sudo make install
```

4. configure and build the project code using cmake

