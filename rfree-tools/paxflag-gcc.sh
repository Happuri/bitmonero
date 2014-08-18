#!/bin/bash

set -x
setfattr -n user.pax.flags -v "rm" $HOME/.local/libexec/gcc/x86_64-unknown-linux-gnu/4.8.3/cc1plus

