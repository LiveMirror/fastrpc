cur_dir=`pwd`;
cd ependingpool
make clean
make
cd $cur_dir

cd http_codec
make clean
make
cd $cur_dir

cd svr
make clean
make
cd $cur_dir

cd comm
make clean
make
cd $cur_dir

cd xcore
make clean
make
cd $cur_dir

cd http_rpc_client
make clean
make
cd $cur_dir

cd http_rpc_server
make clean
make
cd $cur_dir

