#!/bin/bash  
set -x
echo "# setting for bitmonero " >> $HOME/.bashrc 
echo 'export CC="ccache $HOME/.local/bin/gcc" ; export CXX="ccache $HOME/.local/bin/g++" ; export CPP="$HOME/.local/bin/gcc -o" ' >> $HOME/.bashrc
echo 'export PKG_CONFIG_PATH=$HOME/.local/lib/pkgconfig:$PKG_CONFIG_PATH ' >> $HOME/.bashrc
echo 'export BOOST_ROOT=$HOME/.local ' >> $HOME/.bashrc
echo 'export LD_LIBRARY_PATH="$HOME/.local/lib:$HOME/.local/lib64" ' >> $HOME/.bashrc
echo 'export LC_ALL=C' >> $HOME/.bashrc
echo 'export CCACHE_SLOPPINESS=time_macros' >> $HOME/.bashrc

