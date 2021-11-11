// g++ -g -rdynamic  test_crash.cpp -o test_crash -pthread -std=c++11
#include <iostream>
#include <utility>
#include <thread>
#include <chrono>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <execinfo.h>


#define BACKTRACE_SIZE 16

void ShowStack(void)
{
	int i;
	void *buffer[BACKTRACE_SIZE];

	int n = backtrace(buffer, BACKTRACE_SIZE);
	printf("[%s]:[%d] n = %d\n", __func__, __LINE__, n);
	char **symbols = backtrace_symbols(buffer, n);
	if(NULL == symbols){
		perror("backtrace symbols");
		exit(EXIT_FAILURE);
	}
	printf("[%s]:[%d]\n", __func__, __LINE__);
	for (i = 0; i < n; i++) {
		printf("%d: %s\n", i, symbols[i]);
	}

	free(symbols);
}

void sigsegv_handler(int signo)
{
	if (signo == SIGSEGV) {
		printf("Receive SIGSEGV signal\n");
		printf("-----call stack-----\n");
		ShowStack();
		exit(-1);
	} else {
		printf("this is sig %d", signo);
	}
}

void f1(int n)
{
    for (int i = 0; i < 100; ++i) {
        std::cout << "Thread 1 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void f2(int& n)
{
    for (int i = 101; i < 150; ++i) {
        std::cout << "Thread 2 executing\n";
        ++n;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

extern "C" void crash();
void crash()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    char *p = NULL;
    *p = 0x55;
}

int main()
{
    signal(SIGSEGV, sigsegv_handler);

    int n = 0;
    std::thread t1; // t1 is not a thread
    std::thread t2(f1, n + 1); // pass by value
    std::thread t3(f2, std::ref(n)); // pass by reference
    std::thread t4(std::move(t3)); // t4 is now running f2(). t3 is no longer a thread
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread t7(crash);
    t2.join();
    t4.join();
}