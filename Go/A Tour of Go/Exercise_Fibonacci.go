package main

import "fmt"

func fibonacci() func() int {
	res_pre := 0
	res := 1
	return func() int {
		res_pre, res = res, res_pre + res
		return res - res_pre
	}
}

func main() {
	f := fibonacci()
	for i := 0; i < 10; i++ {
		fmt.Println(f())
	}
}
