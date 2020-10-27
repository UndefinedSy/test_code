// 练习：切片
// 实现 Pic。它应当返回一个长度为 dy 的切片，其中每个元素是一个长度为 dx，元素类型为 uint8 的切片。当你运行此程序时，它会将每个整数解释为灰度值（好吧，其实是蓝度值）并显示它所对应的图像。

package main

import (
	"golang.org/x/tour/pic";
	"math";
)

func func1(x, y int) uint8 {
	return uint8((x+y)/2)
}

func func2(x, y int) uint8 {
	return uint8(x*y)
}

func func3(x, y int) uint8 {
	return uint8(x^y)
}

func func4(x, y int) uint8 {
	return uint8(float64(x) * math.Log(float64(y)))
}

func func5(x, y int) uint8 {
	return uint8(x%(y+1))
}

func Pic(dx, dy int) [][]uint8 {
	res := make([][]uint8, dy)

	for i := range res {
		res[i] = make([]uint8, dx)
		for j := range res[i] {
			res[i][j] = func5(i, j)
		}
	}
	
	return res
}

func main() {
	pic.Show(Pic)
}
