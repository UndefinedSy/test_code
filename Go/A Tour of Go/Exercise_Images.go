package main

import (
	"golang.org/x/tour/pic"
	"image"
	"image/color"
	"math"
)

type Image struct{}

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

func (img Image) Bounds() image.Rectangle {
	return image.Rect(0, 0, 114, 514)
}

func (img Image) ColorModel() color.Model {
	return color.RGBAModel
}

func (img Image) At(x int, y int) color.Color {
	// v := func5(x, y)
	return color.RGBA{uint8(x), uint8(y), 255, 255}
}

func main() {
	m := Image{}
	pic.ShowImage(m)
}
