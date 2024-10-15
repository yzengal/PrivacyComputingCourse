#!/bin/bash

ORIGINAL_DIR=$(pwd)
cd ../build
name=Tom
n=10
dim=4

./user --ip-file=../configuration/ip.txt --name=$name --n=$n --dim=$dim

if [ $? -ne 0 ]; then  
    echo "Query user ${name} FAIL"  
    exit 1  
fi 

cd "$ORIGINAL_DIR"
