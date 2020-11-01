package crc

import "strings"

func Check(codeword, generator string) bool{
	quotient := mod2div(codeword, generator)
	return !strings.Contains(quotient, "1")
}
