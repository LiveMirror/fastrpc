
#protobuf
#ullib
#mongo-driver
#epoll_test_uu


CUR_DIR=`pwd`

cd ependingpool
make clean
make
cd $CUR_DIR

cd http_codec
make clean
make
cd $CUR_DIR

cd svr
make clean
make
cd $CUR_DIR

cd ipcs_common
make clean
make
cd $CUR_DIR

cd comm
make clean
make
cd $CUR_DIR

cd xcore
make clean
make
cd $CUR_DIR

cd arpc_client_lib
make clean
make
cd $CUR_DIR

cd arpc_server
make clean
make
cd $CUR_DIR

cd arpc_client_for_pc
make clean
make
cd $CUR_DIR

cd arpc_client_for_server
make clean
make
cd $CUR_DIR
