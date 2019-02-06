CXXFLAGS=-std=c++17 -g

default: ipk-client ipk-server

ipk-client:client.cc Protocol.o
	$(CXX) ${CXXFLAGS} client.cc Protocol.o -o $@

ipk-server:server.cc Protocol.o
	$(CXX) ${CXXFLAGS} server.cc Protocol.o -o $@ 

Protocol.o:Protocol.cc
	$(CXX) ${CXXFLAGS} -o $@ -c Protocol.cc
