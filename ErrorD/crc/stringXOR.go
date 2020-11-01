package crc

import "strings"

type myError struct{
	message string
}

func (m* myError) Error() string{
	return m.message
}

//StringXOR returns the XOR of 2 Strings
func StringXOR(s1, s2 string) (string, error){
	if len(s1) != len(s2){
		return "", &myError{message: "Length is mismatching"}
	}

	var xor []string
	for i := range s1{
		if s1[i] == s2[i]{
			xor = append(xor, "0")
		} else {
			xor = append(xor, "1")
		}
	}

	return strings.Join(xor, ""), nil
}
