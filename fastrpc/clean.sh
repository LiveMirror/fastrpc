cur_dir=`pwd`;
cd ependingpool
make clean
cd $cur_dir

cd http_codec
make clean
cd $cur_dir

cd svr
make clean
cd $cur_dir

cd comm
make clean
cd $cur_dir

cd xcore
make clean
cd $cur_dir

cd http_rpc_client
make clean
cd $cur_dir

cd http_rpc_server
make clean
cd $cur_dir
