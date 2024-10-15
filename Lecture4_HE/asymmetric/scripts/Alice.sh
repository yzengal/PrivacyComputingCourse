#!/bin/bash

ORIGINAL_DIR=$(pwd)
cd ../build
silo_id=1
port=$((50050 + ${silo_id}))
n=10
dim=4
name=Alice

./holder --id=$silo_id --ip=localhost --port=$port --name=$name --n=$n --dim=$dim

if [ $? -ne 0 ]; then  
    echo "Data Holder #${silo_id} ${name} FAIL"  
    exit 1  
fi 

cd "$ORIGINAL_DIR"