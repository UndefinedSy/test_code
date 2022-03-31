package main

import (
	"fmt"
	"os"
	"syscall"
	"time"
	"unsafe"

	"golang.org/x/sys/unix"
)

const defaultMaxFileSize = 1 << 32 // 假设文件最大为 1G

type MMappedFile struct {
	file     *os.File
	Data     *[defaultMaxFileSize]byte
	dataref  []byte
	readOnly bool
}

func NewMMappedFile(f *os.File, isReadOnly bool) *MMappedFile {
	return &MMappedFile{
		file:     f,
		readOnly: isReadOnly,
	}
}

func (m *MMappedFile) Mmap(sz int) error {
	// Map the data file to memory.
	prot := syscall.PROT_READ
	if !m.readOnly {
		prot |= syscall.PROT_WRITE
	}
	b, err := unix.Mmap(int(m.file.Fd()), 0, sz, prot, syscall.MAP_SHARED)
	if err != nil {
		return err
	}

	if m.readOnly {
		if err = unix.Madvise(b, syscall.MADV_SEQUENTIAL); err != nil {
			return err
		}
	}

	// Save the original byte slice and convert to a byte array pointer.
	m.dataref = b
	m.Data = (*[defaultMaxFileSize]byte)(unsafe.Pointer(&b[0]))

	return nil
}

func (m *MMappedFile) Munmap() error {
	// Ignore the unmap if we have no mapped data.
	if m.dataref == nil {
		return nil
	}

	// Unmap using the original byte slice.
	err := syscall.Munmap(m.dataref)
	m.dataref = nil
	m.Data = nil
	return err
}

func main() {
	f, _ := os.OpenFile("/data/ouwei/go/src/DRC/usage_test/replicator/data/tmp_binlog",
		os.O_CREATE|os.O_RDWR, 0644)

	mfile := NewMMappedFile(f, true)

	filesz := 1 << 10

	if err := mfile.Mmap(int(filesz)); err != nil {
		fmt.Printf("Failed to mmap file with error: %s", err)
		panic(err)
	}

	for {
		fmt.Printf("first 100 bytes: [%v]", mfile.Data[:100])
		time.Sleep(1 * time.Second)
	}

	// msg := "hello world!"

	// demo.grow(int64(len(msg) * 2))
	// for i, v := range msg {
	// 	demo.data[2*i] = byte(v)
	// 	demo.data[2*i+1] = byte(' ')
	// }
	// time.sleep(60*time.Second)

	// demo.munmap()
}
