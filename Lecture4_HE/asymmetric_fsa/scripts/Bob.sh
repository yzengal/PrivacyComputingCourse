#!/bin/bash

ORIGINAL_DIR=$(pwd)
cd ../build
holder_id=2
port=$((50050 + ${holder_id}))
n=500
dim=128
name=Bob

./holder --id=$holder_id --ip=localhost --port=$port --name=$name --n=$n --dim=$dim

if [ $? -ne 0 ]; then  
    echo "Data Holder #${holder_id} ${name} FAIL"  
    exit 1  
fi 

cd "$ORIGINAL_DIR"