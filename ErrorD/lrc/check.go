package lrc

//Check if the sequence is correct or not.
func Check(raw string) (bool,string) {
	data := padding(&raw)

	for i := 0; i < MAXBITSLENGTH; i++ {
		parity := 0
		for j := 0; j < len(data); j+=MAXBITSLENGTH {
			parity ^= int(data[i+j]) - '0'
		}
		if parity != 0 {
			return false, raw[:len(raw)-MAXBITSLENGTH]
		}
	}
	return true, raw[:len(raw)-MAXBITSLENGTH]
}
