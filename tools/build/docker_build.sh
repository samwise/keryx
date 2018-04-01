#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [ -f /.dockerenv ]; then
    cd $DIR/build/clang_debug && ninja
else
    cid=`docker ps | grep keryx | awk '{print $1}'`
    docker exec $cid /bin/bash -c "cd build/clang_debug/ && ninja"
fi
