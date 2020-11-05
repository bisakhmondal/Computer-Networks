package lrc

import (
	"math"
	"strconv"
	"strings"
)

const (
	NAME = "LRC"
	MAXBITSLENGTH = 8
)
func padding(data *string) string {
	mulof8 := math.Ceil(float64(len(*data)) / MAXBITSLENGTH)
	initialZeros := MAXBITSLENGTH*int(mulof8) - len(*data)

	return strings.Repeat("0", initialZeros) + *data
}

//Generate generates codeword after applying LRC
func Generate(raw string) string{

	padded := padding(&raw)

	parities := ""
	for i := 0; i < MAXBITSLENGTH; i++ {
		tempP := 0
		for j := 0; j < len(padded); j+=MAXBITSLENGTH {
			tempP ^= int(padded[i+j]) - '0'
		}
		parities += strconv.Itoa(tempP)
	}
	return raw + parities
}

func GenerateAll(datawords [] string) [] string{
	codewords := make([] string, len(datawords))
	for i := range datawords{
		codewords[i] = Generate(datawords[i])
	}
	return codewords
}