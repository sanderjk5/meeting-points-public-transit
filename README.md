# meeting-points-public-transit
Project for my master thesis. The goal is to implement an algorithm that solves the meeting points problem for public transit networks.

## Setup 
Prerequesites: git lfs, cmake

1. Clone the repository
```bash
git clone https://github.com/sanderjk5/meeting-points-public-transit.git
cd meeting-points-public-transit
```
2. Build the project:
```bash
rm -rf build && mkdir build
git submodule init && git submodule update
cd build
cmake ..
make && make install
cd ..
```
3. Start the program:
```bash
bin/meeting-points-public-transit dataType mode
```
The program can be started with two additional arguments dataType and experiment:
1. The dataType argument specifies which public transit network should be used and can take the following values:

  * vvs: The whole public transit network of Stuttgart (default value).
  * de: The whole public transit network of germany.

2. The mode argument specifies what is executed. It can take the following values:

   * <none>: The meeting point cli is executed. The user can question meeting point queries.
   * eat: The earliest arrival time cli is executed. The user can question earliest arrival time queries.
   * exp: The experiments are executed.
