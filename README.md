# meeting-points-public-transit
Project for my master thesis. The goal is to implement an algorithm that solves the meeting points problem for public transit networks.

## Setup 
Prerequesites: git lfs, cmake

1. Clone the repository
```bash
git clone https://github.com/sanderjk5/meeting-points-public-transit.git
cd meeting-points-public-transit
```
2. Build and start the program:
```bash
rm -rf build && mkdir build
git submodule init && git submodule update
cd build
cmake ..
make & make install
cd ..
bin/meeting-points-public-transit
```
