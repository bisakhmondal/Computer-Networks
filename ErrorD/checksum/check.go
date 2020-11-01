package checksum

import (
	"log"
	"strings"
)

//Check checks codeword with the checksum if the whole codeword is valid or not.
func Check(codeword string) bool{
	padded := padding(&codeword)

	splitsData := splits(padded)
	checksum  := splitsData[0]
	for i:=1 ; i<len(splitsData) ; i++{
		Tchecksum, carry, err := partialSUM(splitsData[i], checksum)
		// log.Println(i, checksum, carry)
		if err!=nil{
			log.Fatal(err)
		}
		if carry!=0{
			Tchecksum = addLSBCarry(Tchecksum, carry)
		}
		checksum = Tchecksum
	}
	return !strings.Contains(checksum,"0")
}