package crc

import "strings"

const NAME ="CRC"

//Generate generates codeword after applying CRC
func Generate(raw , generator string) string{

	dummyCRC := raw + strings.Repeat("0", len(generator)-1)
	quotient := mod2div(dummyCRC, generator)	

	return raw+quotient
}
