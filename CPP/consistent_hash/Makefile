test_consistent: consistent.cpp test_consistent.cpp
	clang++ -o test_consistent consistent.cpp test_consistent.cpp

test_rpc_server: test_rpc_server.cpp server.cpp
	clang++ -o test_rpc_server test_rpc_server.cpp server.cpp -pthread

test_rpc_client: test_rpc_client.cpp client.cpp
	clang++ -o test_rpc_client test_rpc_client.cpp client.cpp

clean:
	rm *.o