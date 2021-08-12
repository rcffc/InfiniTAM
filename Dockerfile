FROM nvidia/cuda:11.2.0-cudnn8-devel-ubuntu20.04
# docker pull nvidia/cuda:11.2.0-cudnn8-devel-ubuntu20.04
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get update && apt-get install -y cmake build-essential
RUN apt-get update && apt-get install -y freeglut3-dev
RUN apt-get update && apt-get install -y software-properties-common
RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt-get update && apt-get install -y python3.7 python3-pip
RUN apt-get update && apt-get install -y libpng-dev
# WORKDIR /infinitam/build
# RUN cmake .. -DWITH_CUDA:BOOL=TRUE -DWITH_PNG:BOOL=TRUE -DPNG_PNG_INCLUDE_DIR=/usr/include -DPNG_LIBRARY=/usr/lib/x86_64-linux-gnu/libpng.so
# RUN make

WORKDIR /pynfinitam
RUN python3.7 -m pip install virtualenv
RUN python3.7 -m virtualenv venv
RUN . venv/bin/activate

RUN apt-get update && apt-get install libpng-dev python3-dev 
# apt install libpython3.7-dev
WORKDIR /pynfinitam/build
# cmake .. -Dpybind11_DIR:STRING=venv/lib/python3.7/site-packages/share/cmake/pybind11/ -DBOOST_ROOT:STRING=/spaint/libraries/boost_1_58_0/ -DBOOST_INCLUDE_DIR:STRING=/spaint/libraries/boost_1_58_0/include/boost/ -DEIGEN_INCLUDE_DIR:STRING=/spaint/libraries/Eigen-3.2.2