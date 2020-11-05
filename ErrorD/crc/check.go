package crc

import "strings"

func Check(codeword, generator string) (bool, string){
	syndrome := mod2div(codeword, generator)

	return !strings.Contains(syndrome, "1"), codeword[:len(codeword)-len(generator)-1]
}
