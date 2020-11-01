package main

import (
	"ErrorD/vrc"
	"ErrorD/lrc"
	chk "ErrorD/checksum"
	"ErrorD/crc"
	"flag"
	"io/ioutil"
	"log"
	"os"
	"math/rand"
	"sync"
)

var (
	errorBits = flag.Int("err", 0, "inject error in n bits")
	scheme    = flag.String("scheme", "lrc", "detection schme used on codeword")
	filename  = flag.String("file", "data.txt", "File to be read for input codeword")
	lrcScheme = flag.String("CRC", "CRC7", "the generator scheme for cyclic redundancy checks")
	wg sync.WaitGroup
	NUMSender int
)

func init(){
	flag.Parse()

	if *scheme =="all"{
		//4 schemes x2
		NUMSender = 8

		wg.Add(NUMSender)
	}else{
		NUMSender = 2
		wg.Add(NUMSender)
	}
	rand.Seed(60)
}

type responseType struct{
	codeword, schemeName, generator string
	checker func(string)bool
}

func main() {

	file, err := os.Open(*filename)
	if err != nil {
		log.Fatal("Invalid Filename")
	}

	dataBytes, _ := ioutil.ReadAll(file)
	dataword := string(dataBytes)

	response := make(chan *responseType,2)
	
	if *scheme == "all"{
		go Sender(dataword, "lrc", response)
		go Sender(dataword, "vrc", response)
		go Sender(dataword, "checksum", response)
		go Sender(dataword, "crc", response)
	}else {
		go Sender(dataword, *scheme, response)
	}

	for i := 0; i< NUMSender/2; i++{
		
		select{
		case encodedCodeword := <- response:
			encodedCodeword.codeword = taintBits(encodedCodeword.codeword)
			go Receiver(encodedCodeword)
		}
	}
	wg.Wait()
	test2(dataword)
	test3(dataword)
}

func Sender(dataword, localScheme string, chanvar chan *responseType){
	defer wg.Done()
	var codeword string
	response := &responseType{}
	
	switch localScheme{
	case "lrc":
		codeword = lrc.Generate(dataword)
		response.checker = lrc.Check
		response.schemeName = lrc.NAME

	case "vrc":
		codeword = vrc.Generate(dataword)
		response.checker = vrc.Check
		response.schemeName = vrc.NAME

	case "checksum":
		codeword = chk.Generate(dataword)
		response.checker = chk.Check
		response.schemeName = chk.NAME

	case "crc":
		var generator string
		switch *lrcScheme{
		case "CRC1":
			generator = crc.CRC1
		case "CRC7":
			generator = crc.CRC7
		case "CRC16":
			generator = crc.CRC16
		default:
			generator = crc.CRC8
		}
		codeword = crc.Generate(dataword, generator)
		response.generator = generator
		response.schemeName = crc.NAME
	}
	response.codeword= codeword
	chanvar <- response
}


func Receiver(response *responseType){
	defer wg.Done()
	var validation bool 
	if response.schemeName== "CRC"{
		validation = crc.Check(response.codeword, response.generator)
	} else{
		validation = response.checker(response.codeword)
	}
	log.Println("Test: ", response.schemeName, fmtTest(validation))
}

//Inject error at random position in codeword.
func taintBits(codeword string) string{
	errbitsTemp := *errorBits
	codearr := []rune(codeword)
	for errbitsTemp > 0{
		idx := rand.Intn(len(codeword))
		codearr[idx] = rune(int(codearr[idx]-'0')^1 +'0')
		errbitsTemp--
	}
	return string(codearr)
}

//Formatting tool to print custom errors as per the requirement.
func fmtTest(result bool)string {
	log.Println("Test Running...")

	if result{
		return "PASS"
	}
	return "FAIL"
}

func test2(dataword string){
	log.Println("Error is detected by checksum but not by CRC.")
	codeword := crc.Generate(dataword, crc.CRC8)
	codecodeword := chk.Generate(codeword)
	log.Println("dataword: ", dataword)
	log.Println("Codeword: ", codecodeword)
	log.Println("CHECKSUM ", fmtTest(chk.Check(codecodeword)))
	log.Println("CRC ", fmtTest(crc.Check(codecodeword, crc.CRC8)))
}

func test3(dataword string){
	log.Println("Error is detected by VRC but not by CRC.")
	codeword := vrc.Generate(dataword)
	codecodeword := crc.Generate(codeword, crc.CRC7)
	log.Println("dataword: ", dataword)
	log.Println("Codeword: ", codecodeword)
	log.Println("VRC ", fmtTest(vrc.Check(codecodeword)))
	log.Println("CRC ", fmtTest(crc.Check(codecodeword, crc.CRC7)))
}