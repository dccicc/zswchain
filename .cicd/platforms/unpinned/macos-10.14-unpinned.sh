#!/bin/bash
set -eo pipefail
VERSION=1
brew update
<<<<<<< HEAD
brew install git cmake python libtool libusb graphviz automake wget gmp pkgconfig doxygen openssl@1.1 jq boost libpq postgres || :
# libpqxx 7.3+ installations on mojave try to import libs not present in the sdk. pin to libpqxx 7.2.1 instead.
curl -LO  https://raw.githubusercontent.com/Homebrew/homebrew-core/d14398187084e1d3fd1763ec13cea1044946a51f/Formula/libpqxx.rb
brew install -f ./libpqxx.rb
# install nvm for ship_test
cd ~ && brew install nvm && mkdir -p ~/.nvm && echo "export NVM_DIR=$HOME/.nvm" >> ~/.bash_profile && echo 'source $(brew --prefix nvm)/nvm.sh' >> ~/.bash_profile && cat ~/.bash_profile && source ~/.bash_profile && echo $NVM_DIR && nvm install --lts=dubnium
# initialize postgres configuration files
sudo rm -rf /usr/local/var/postgres
initdb --locale=C -E UTF-8 /usr/local/var/postgres
=======
brew install git icu4c python libtool libusb graphviz automake wget gmp llvm@7 pkgconfig doxygen openssl@1.1 cmake jq boost || :
# install mongoDB
cd ~
curl -OL https://fastdl.mongodb.org/osx/mongodb-osx-ssl-x86_64-3.6.3.tgz
tar -xzf mongodb-osx-ssl-x86_64-3.6.3.tgz
rm -f mongodb-osx-ssl-x86_64-3.6.3.tgz
ln -s ~/mongodb-osx-x86_64-3.6.3 ~/mongodb
# install mongo-c-driver from source
cd /tmp
curl -LO https://github.com/mongodb/mongo-c-driver/releases/download/1.13.0/mongo-c-driver-1.13.0.tar.gz
tar -xzf mongo-c-driver-1.13.0.tar.gz
cd mongo-c-driver-1.13.0
mkdir -p cmake-build
cd cmake-build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX='/usr/local' -DENABLE_BSON=ON -DENABLE_SSL=DARWIN -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF -DENABLE_STATIC=ON -DENABLE_ICU=OFF -DENABLE_SASL=OFF -DENABLE_SNAPPY=OFF ..
make -j $(getconf _NPROCESSORS_ONLN)
sudo make install
cd ../..
rm mongo-c-driver-1.13.0.tar.gz
# install mongo-cxx-driver from source
cd /tmp
curl -L https://github.com/mongodb/mongo-cxx-driver/archive/r3.4.0.tar.gz -o mongo-cxx-driver-r3.4.0.tar.gz
tar -xzf mongo-cxx-driver-r3.4.0.tar.gz
cd mongo-cxx-driver-r3.4.0/build
cmake -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX='/usr/local' ..
make -j $(getconf _NPROCESSORS_ONLN) VERBOSE=1
sudo make install
cd ../..
rm -f mongo-cxx-driver-r3.4.0.tar.gz 
>>>>>>> 1926c9588e98187e7666c14ab94ac800a6ca84d7
