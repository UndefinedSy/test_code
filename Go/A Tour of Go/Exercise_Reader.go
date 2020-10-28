package main

import (
	"golang.org/x/tour/reader"
)

type MyReader struct{}

// TODO: 给 MyReader 添加一个 Read([]byte) (int, error) 方法

func (m_r MyReader) Read(array []byte) (int, error) {
	for i := range array {
		array[i] = 'A'
	}
	return len(array), nil
}

func main() {
	reader.Validate(MyReader{})
}
