#!/bin/bash

if [ "$#" -ne 4 ]
then
    echo "Usage: build.sh <git-user> <git-password> <version> <branch>"
    exit -1
fi
CWD=$(dirname "$(readlink -f $0)") && cd $CWD && \
docker build --rm -t paicoin-build -f Dockerfile.build . && \
rm -f *.deb && \
docker run -e GIT_USER=$1 -e GIT_PASSWD=$2 -e VERSION=$3 -e BRANCH=$4 --rm --volume $CWD:/paicoin -i -t paicoin-build && \
docker build --rm -t $4:$3 -f Dockerfile .
