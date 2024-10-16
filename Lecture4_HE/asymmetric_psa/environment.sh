#!/bin/bash
export MY_GRPC_PATH="/opt/gRPC/"
export MY_BOOST_PATH="/opt/boost/"
export MY_SEAL_PATH="/opt/SEAL/"
export PATH="${MY_GRPC_PATH}/bin:$PATH"
export LD_LIBRARY_PATH="${MY_GRPC_PATH}/lib:${MY_BOOST_PATH}/lib::${MY_SEAL_PATH}/lib:$LD_LIBRARY_PATH"
export C_INCLUDE_PATH="${MY_GRPC_PATH}/include:${MY_BOOST_PATH}/include:${MY_SEAL_PATH}/include:$C_INCLUDE_PATH"
export CPLUS_INCLUDE_PATH="${MY_GRPC_PATH}/include:${MY_BOOST_PATH}/include:${MY_SEAL_PATH}/include:$CPLUS_INCLUDE_PATH"
