mkdir -p build
cd build
cmake -A x64 -DCMAKE_BUILD_TYPE=Debug ..

if [ $? != 0 ]; then
	read _
fi
