package vrc

import (
	. "ErrorD/utils"
	"log"
)

//Check if the sequence is correct or not.
func Check( codeword string) (bool, string){
	ones := 0
	for _ , i := range codeword{
		if i!='0' && i!='1'{
			log.Fatal(MyError{"Invalid Char"})	
		}
		ones += int(i-'0')
	}

	return ones%2==0, codeword[:len(codeword)-1]
}

