package checksum

import "strings"

func flipBits(data string) string{
	flips := make([]string, len(data))
	for i, d := range data{
		flips[i] = string((d-'0')^1 + '0')
	}
	return strings.Join(flips, "")
}
