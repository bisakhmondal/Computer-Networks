package crc

import "testing"

func TestXORTestPass(t *testing.T){
	str1, str2, ans := "01101","10011", "11110"
	
	xor, err := StringXOR(str1, str2)

	if xor!=ans || err!= nil{
		t.Fatal("XOR test failed ", xor, ans, err )
	}
}

func TestXORtestWRONGLEN(t *testing.T){
	str1, str2, ans := "011011","10011", "11110"
	
	xor, err := StringXOR(str1, str2)

	if xor!=ans || err!= nil{
		t.Fatal("XOR test failed ", xor, ans, err )
	}
}
