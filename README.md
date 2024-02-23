# meeting-points-public-transit
Project for my master thesis. The goal is to implement an algorithm that solves the meeting points problem for public transit networks.

## Setup 
Prerequesites: git lfs, cmake

1. Clone the repository
```bash
git clone https://github.com/sanderjk5/meeting-points-public-transit.git
cd meeting-points-public-transit
```
2. Build the program:
```bash
rm -rf build && mkdir build
git submodule init && git submodule update
cd build
cmake ..
make && make install
cd ..
bin/meeting-points-public-transit dataType experiment
```
3. Start the program:
```bash
bin/meeting-points-public-transit dataType experiment
```
The program can be started with two additional arguments dataType and experiment:
1. The dataType argument specifies which public transport network should be used and can take the following values:

  * vvs: The whole public transportation network of Stuttgart (default value).
  * fern: The long-distance public transportation network of germany.
  * regio: The regional public transportation network of germany without busses and underground trains.
  * fern_regio: The combination of regional and long-distance public transportation network of germany.
  * de: The whole public transportation network of germany.

2. The experiment argument is a flag that determines if the program should run the experiments or answer queries. It has a single value: exp. If this value is given the flag is set, otherwise it isn't.
