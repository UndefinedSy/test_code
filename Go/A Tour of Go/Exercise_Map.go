package main

import (
	"golang.org/x/tour/wc";
	"strings";
)

func WordCount(s string) map[string]int {
	res := make(map[string]int)
	for _, v := range(strings.Fields(s)) {
		res[v]++
	}
	
	return res
}

func main() {
	wc.Test(WordCount)
}
