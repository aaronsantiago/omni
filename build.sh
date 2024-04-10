mkdir build
cd ./build
cmake /code -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCPACK_GENERATOR="DEB" -DOpenGL_GL_PREFERENCE=LEGACY
make -j6
cpack
./bin/yuri2 -h