#!/bin/bash

rc --compile clang++ $@
/usr/lib/ccache/clang++ $@
