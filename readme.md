mkdir build 
cd build
cmake -DCMAKE_BUILD_TYPE=Debug  -DCMAKE_PREFIX_PATH=/home/ec2-user/libtorch/libtorch  -DBUILD_EXAMPLES=True ..
