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
	"strings"
	"time"
)

var (
	errorBits = flag.Int("err", -1, "inject error in n bits")
	scheme    = flag.String("scheme", "lrc", "detection schme used on codeword")
	filename  = flag.String("file", "data.txt", "File to be read for input codeword")
	crcScheme = flag.String("CRC", "CRC8", "the generator scheme for cyclic redundancy checks")
	frameSize = flag.Int("framesize", 10, "the framesize on ehich data will be splited")
	verbose = flag.Bool("verbose", true , "the test results")
	opFile = "output.txt"
	fileForRead = false
	wg sync.WaitGroup
	fileMut sync.Mutex
	// opFilemap = map[string]string{
	// 	"LRC": "codewords/lrc.txt",
	// 	"VRC":  "codewords/vrc.txt",
	// 	"CRC":  "codewords/crc.txt",
	// 	"CHECKSUM":  "codewords/checksum.txt",
	// }

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
	
}

type responseType struct{
	schemeName, generator string
	c []string
	checker func(string) (bool, string)
}

func main() {
	
	rand.Seed(time.Now().UnixNano())
	// log.Println(rand.Intn(15))
	file, err := os.Open(*filename)
	if err != nil {
		log.Fatal("Invalid Filename")
	}

	dataBytes, _ := ioutil.ReadAll(file)
	inputString := string(dataBytes)


	response := make(chan *responseType,1)
	
	if *scheme == "all"{
		go Sender(inputString, "lrc", response)
		go Sender(inputString, "vrc", response)
		go Sender(inputString, "checksum", response)
		go Sender(inputString, "crc", response)
	}else {
		go Sender(inputString, *scheme, response)
	}

	for i := 0; i< NUMSender/2; i++{
		
		select{
		case deliveryStatus := <- response:
			
			fileMut.Lock()
			fss ,_ := os.Open(opFile)//map[deliveryStatus.schemeName])
			readBytes ,_ := ioutil.ReadAll(fss)
			codewords := strings.Split(string(readBytes), " ")
			fileForRead=false
			fileMut.Unlock()

			// log.Println(codewords)
			taintBits(codewords) // adding transmission error
			// log.Println(codewords)

			go Receiver(codewords, deliveryStatus, *verbose)
		}
	}
	wg.Wait()
	test2(inputString[0:8])
	test3(inputString[0:8])
}

func Sender(inputString, localScheme string, chanvar chan *responseType){
	defer wg.Done()
	var codeword []string
	response := &responseType{}
	
	datawords := splitFrame(inputString)

	switch localScheme{
	case "lrc":
		codeword = lrc.GenerateAll(datawords)
		response.checker = lrc.Check
		response.schemeName = lrc.NAME

	case "vrc":
		codeword = vrc.GenerateAll(datawords)
		response.checker = vrc.Check
		response.schemeName = vrc.NAME

	case "checksum":
		codeword = chk.GenerateAll(datawords)
		response.checker = chk.Check
		response.schemeName = chk.NAME

	case "crc":
		var generator string
		switch *crcScheme{
		case "CRC1":
			generator = crc.CRC1
		case "CRC7":
			generator = crc.CRC7
		case "CRC16":
			generator = crc.CRC16
		default:
			generator = crc.CRC8
		}
		codeword = crc.GenerateAll(datawords, generator)
		response.generator = generator
		response.schemeName = crc.NAME
	}
	
	fileMut.Lock()
	fi, _ := os.Create(opFile)//map[response.schemeName])
	fi.WriteString(strings.Join(codeword, " "))
	fi.Close()
	fileForRead=true
	fileMut.Unlock()

	response.c = codeword
	chanvar <- response
}


func Receiver(codewords []string, response *responseType, verbose bool){
	defer wg.Done()
	falseCounter, dataword :=0, ""
	for _, codeword := range codewords{
		var validation bool
		var data string
		if response.schemeName== "CRC"{
			validation, data = crc.Check(codeword, response.generator)
		} else{
			validation, data = response.checker(codeword)
		}
		if !validation{
			falseCounter++
		}
		dataword += data
	}
	log.Println("Test: ", response.schemeName, fmtTest(falseCounter==0))
	if verbose{
		log.Println(response.schemeName, " : Received Data: ", dataword)
		log.Println(response.schemeName, "Total noisy frames: ", falseCounter, " out of: ", len(codewords) )
	}
}

//Inject error at random position random number of times in codeword.
func taintBits(codewords []string) {
	noiseCnt:=0
	for i := range codewords{

		codeword := codewords[i]
		errbitsTemp := *errorBits

		if errbitsTemp ==-1{
			errbitsTemp = rand.Intn(len(codeword))
		}
		if errbitsTemp!=0{
			noiseCnt++
		}
		codearr := []rune(codeword)
		for errbitsTemp > 0{
			idx := rand.Intn(len(codeword))
			codearr[idx] = rune(int(codearr[idx]-'0')^1 +'0')
			errbitsTemp--
		}
		codewords[i] =  string(codearr)
	}
	log.Println("Noisy Frame on Transmission: ", noiseCnt)
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
	log.Println("TEST2: Error is detected by checksum but not by CRC.")
	log.Println("dataword: ", dataword)
	codeword := crc.Generate(dataword, crc.CRC7)
	berrorCRC := BurstError(codeword, len(crc.CRC7))
	log.Println("CRC Codeword: True: ",codeword, " : Erroneous: ",  berrorCRC)
	validation,_ := crc.Check(berrorCRC, crc.CRC7)
	log.Println("CRC ", fmtTest(validation))

	codeword = chk.Generate(dataword)
	berrorCRC = BurstError(codeword, len(crc.CRC7))
	validation, _ = chk.Check(berrorCRC)
	log.Println("CHECKSUM Codeword: True: ",codeword, " : Erroneous: ",  berrorCRC)
	log.Println("CHECKSUM ", fmtTest(validation))
}


func BurstError(codeword string, len int) string{
	codearr := []rune(codeword)
	codearr[0] = rune(int(codearr[0]-'0')^1 +'0')
	codearr[len-1] = rune(int(codearr[len-1]-'0')^1 +'0')
	mid := rand.Intn(len)
	codearr[mid] = rune(int(codearr[mid]-'0')^1 +'0')
	return string(codearr)
}

func test3(dataword string){
	log.Println("TEST3: Error is detected by checksum but not by CRC.")
	log.Println("dataword: ", dataword)
	codeword := crc.Generate(dataword, crc.CRC7)
	berrorCRC := BurstError(codeword, len(crc.CRC7))
	log.Println("CRC Codeword: True: ",codeword, " : Erroneous: ",  berrorCRC)
	validation,_ := crc.Check(berrorCRC, crc.CRC7)
	log.Println("CRC ", fmtTest(validation))

	codeword = vrc.Generate(dataword)
	berrorCRC = BurstError(codeword, len(crc.CRC7))
	validation, _ = vrc.Check(berrorCRC)
	log.Println("VRC Codeword: True: ",codeword, " : Erroneous: ",  berrorCRC)
	log.Println("VRC ", fmtTest(validation))
}

func splitFrame(str string) []string{
	// for str = ""123456789"" && *frameSize=4 returns [1234 5678 9]

	var chops []string
	i :=0
	for ;i<len(str);i+= *frameSize{
		if len(str[i:])<*frameSize{
			chops = append(chops, str[i:])
			break
		}
		chops = append(chops, str[i: i+ *frameSize])
	}
	return chops
}