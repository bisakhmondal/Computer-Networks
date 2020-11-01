package vrc

import (
	. "ErrorD/utils"
	"log"
)

//Check if the sequence is correct or not.
func Check( data string) bool{
	ones := 0
	for _ , i := range data{
		if i!='0' && i!='1'{
			log.Fatal(MyError{"Invalid Char"})	
		}
		ones += int(i-'0')
	}

	return ones%2==0
}

