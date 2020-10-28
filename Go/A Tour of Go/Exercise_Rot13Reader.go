package main

import (
	"io"
	"os"
	"strings"
)

type rot13Reader struct {
	r io.Reader
}

func rot13(b byte) byte {
	switch {
	// between A and M or between a and m
	case (b > 64 && b < 78) || (b > 96 && b < 110):
		b += 13
	// between N and Z or between n and z
	case (b > 77 && b < 91) || (b > 109 && b < 123):
		b -= 13
	}
	return b
}

func (rot13Reader_ rot13Reader) Read(b []byte) (int, error) {
	n, err := rot13Reader_.r.Read(b)
	if err != nil {
		return n, err
	}
	
	for i := range b {
		b[i] = rot13(b[i])
	}
	return n, err
}

func main() {
	s := strings.NewReader("Lbh penpxrq gur pbqr!")
	r := rot13Reader{s}
	io.Copy(os.Stdout, &r)
}
