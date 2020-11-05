package crc

import "strings"

const NAME ="CRC"

//Generate generates codeword after applying CRC
func Generate(raw , generator string) string{

	dummyCRC := raw + strings.Repeat("0", len(generator)-1)
	rem := mod2div(dummyCRC, generator)	

	return raw+rem
}

func GenerateAll(datawords [] string, generator string) [] string{
	codewords := make([] string, len(datawords))
	for i := range datawords{
		codewords[i] = Generate(datawords[i], generator)
	}
	return codewords
}