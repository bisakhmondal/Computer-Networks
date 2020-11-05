package checksum

import "log"

const (
	MAXBITSLENGTH = 8
	NAME = "CHECKSUM"
)

//Generate generates codeword from dataword by appaendding the 
//checksum calculated from the given dataword.
func Generate(raw string) string{
	padded := padding(&raw)
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

	return raw + flipBits(checksum)
}

func splits(str string) []string{
	var chops []string
	for i:=0;i<len(str);i+=MAXBITSLENGTH{
		chops = append(chops, str[i: i+MAXBITSLENGTH])
	}
	return chops
}

func GenerateAll(datawords [] string) [] string{
	codewords := make([] string, len(datawords))
	for i := range datawords{
		codewords[i] = Generate(datawords[i])
	}
	return codewords
}