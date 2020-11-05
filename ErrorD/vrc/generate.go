package vrc

import (
	"strconv"
	. "ErrorD/utils"
	"log"
)

const NAME = "VRC"

//Generate retruns the  data along with the parity
func Generate(data string) string{
	parity := 0

	for _, i := range data {
		if i!= '0' && i!='1'{
			log.Fatal(MyError{"Invalid Character"})
		}
		parity ^= int(i - '0')
	}
	
	return data + strconv.Itoa(parity)
}

func GenerateAll(datawords [] string) [] string{
	codewords := make([] string, len(datawords))
	for i := range datawords{
		codewords[i] = Generate(datawords[i])
	}
	return codewords
}