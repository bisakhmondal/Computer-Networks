package crc

import "testing"

func TestDiv1(t *testing.T) {
	dividend, divisor, ans := "1010000000", "1001", "011"
	quotient := mod2div(dividend, divisor)
	if quotient != ans {
		t.Fatal("divisor error")
	}
}

func TestDiv2(t *testing.T) {
	dividend, divisor, ans := "1010000011", "1001", "000"
	quotient := mod2div(dividend, divisor)
	if quotient != ans {
		t.Fatal("divisor error")
	}
}
func TestDiv3(t *testing.T) {
	dividend, divisor, ans := "100010110101101001110000", "1001", "111"
	quotient := mod2div(dividend, divisor)
	if quotient != ans {
		t.Fatal("divisor error: ", quotient, ans)
	}
}
