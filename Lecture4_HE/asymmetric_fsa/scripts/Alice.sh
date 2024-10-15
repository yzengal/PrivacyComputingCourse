#!/bin/bash

ORIGINAL_DIR=$(pwd)
cd ../build
silo_id=0
port=$((50051 + ${silo_id}))
n=5
dim=128
name=Alice

./holder --id=$silo_id --ip=localhost --port=$port --name=$name --n=$n --dim=$dim

if [ $? -ne 0 ]; then  
    echo "Data Holder #${silo_id} ${name} FAIL"  
    exit 1  
fi 

cd "$ORIGINAL_DIR"