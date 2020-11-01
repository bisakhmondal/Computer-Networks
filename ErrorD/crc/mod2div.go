package crc

import (
	"strings"
	"log"
)

func mod2div(dividend, divisor string) string{
	//base case
	if len(dividend) <len(divisor){
		return dividend
	}

	dividend = strings.TrimLeft(dividend, "0")
	
	if len(dividend) < len(divisor){
		return strings.Repeat("0", len(divisor)- len(dividend)-1)+dividend
	}

	xorop, err := StringXOR(dividend[0:len(divisor)], divisor)
	if err!=nil{
		log.Fatal(err)
	}
	return mod2div(xorop + dividend[len(divisor):], divisor)
}
