package utils

type InvalidLen struct{}

func (e * InvalidLen) Error() string{
	return "Two Strings of different Length"
}

type MyError struct {
	Message string
}

func (m* MyError) Error() string{
	return m.Message
}
