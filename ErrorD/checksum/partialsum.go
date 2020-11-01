package checksum

import (
	"ErrorD/utils"
	"math"
	"strings"
)

func partialSUM(s1, s2 string) (string, int, error) {
	if len(s1) != len(s2) {
		return "", 0, &utils.InvalidLen{}
	}
	carry := 0
	sum := make([]string, len(s1))
	for i := range s1 {
		idx := len(s1) - i - 1
		temp := int(s1[idx]+s2[idx]-2*'0') + carry
		sum[idx] = string(temp%2 + '0')
		carry = temp / 2
	}
	return strings.Join(sum, ""), carry, nil
}

func addLSBCarry(s string, c int) string {

	sum := make([]string, len(s))
	for i := range s {
		temp := int(s[len(s)-i-1]-'0') + c
		sum[len(s)-i-1] = string(temp%2 + '0')
		c = temp / 2
	}
	return strings.Join(sum, "")
}

func padding(data *string) string {
	mulof8 := math.Ceil(float64(len(*data)) / MAXBITSLENGTH)
	initialZeros := MAXBITSLENGTH*int(mulof8) - len(*data)

	return strings.Repeat("0", initialZeros) + *data
}

