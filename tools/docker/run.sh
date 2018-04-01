#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/../..

docker run \
       --rm \
       --net host \
       -i \
       -h $(hostname)_keryx \
       -v /etc/passwd:/etc/passwd \
       --security-opt seccomp=unconfined \
       -v /var/log/syslog:/var/log/syslog \
       -v  /etc/group:/etc/group \
       -v /tmp/.X11-unix:/tmp/.X11-unix \
       -v /run/user/1000/rdm.socket:/run/user/1000/rdm.socket \
       --shm-size=256m \
       -u $UID \
       -v /home:/home \
       -w $DIR \
       -e DISPLAY=$DISPLAY \
       -t \
       ppinto/keryx \
       sh -c 'rdm --daemon -L ~/temp/rtags_log & /bin/bash' 
