package main

import (
	"flag"
	"fmt"
	"net"
	"reflect"
	"strconv"
	"time"
)

//var node *vomni.Node

var skaits = 0

type Node struct {
	Type     string
	Protocol string
	IP       string
	Port     int
}

var nd *Node

type udp struct {
	node   *Node
	client net.Conn
	server net.Conn
}

var ip string
var msg string
var port int

func init() {

	//	vcli.Init()
	//	flag.Set(vomni.CliProtocolName, vomni.ProtocolUDP)
	//	flag.Set(vomni.CliTypeName, vomni.ConnTypeClient)

	flag.StringVar(&ip, "ip", "127.0.0.1", "internal IP")
	flag.StringVar(&msg, "msg", "", "UDP message")
	flag.IntVar(&port, "port", 1234, "UDP port")

	flag.Parse()

	nd = new(Node)

	nd.Type = "client"
	nd.Protocol = "udp"
	nd.IP = "192.168.1.133"
	nd.Port = 7593
	//	nd.IP = "127.0.0.1"
	//	nd.Port = 1234
	//	nd.cfg()
}

//******* UDP ******
func (c udp) Connect(chErr chan error) {

	err := error(nil)

	ip := c.node.IP
	//port := c.node.Port

	c.client, err = net.Dial("udp", ip+":"+strconv.Itoa(port))

	chErr <- err
}

func (c udp) Receive(str *string, done chan bool, err chan error) {
	done <- true
}

func (c udp) Send(str string, done chan bool, err chan error) {
	done <- true
}

func (c udp) Waiting(done chan bool, err chan error) {
	if "client" == c.node.Type {
		done <- true
	}
}

//******* UDP ******

func main() {

	sutit := []string{"Alex Sitkovecky",
		"Zirgs Bļ...", "Bernijs Dolens",
		"Gorbacheff", "Alexandr Porohovshchikoff"}

	fmt.Printf("%d *** PATS SAKUMS\n%s\n", len(sutit), nd.String())

	messags := flag.Lookup("msg").Value.String()
	ips := flag.Lookup("ip").Value.String()
	//porta := flag.Lookup("port").Value

	addr := ips + ":" + strconv.Itoa(port)

	fmt.Println("SAVIENOJUMA ADRESE ", addr)

	conn, err := net.Dial(nd.Protocol, addr)
	if err != nil {
		fmt.Printf("Connection ERROR: %v\n", err)
		return
	}
	defer conn.Close()

	i := 0
	reizes := 0
	//	chErr := make(chan error)
	//	chDone := make(chan bool)
	for {
		str := messags
		//	str1 := ""

		fmt.Printf("%2d ### Taisos sūtīt: %s\n", i, str)

		_, err = conn.Write([]byte(str))
		buffer := make([]byte, 1024)

		gaidisuAtbildi := false

		if gaidisuAtbildi && nil == err {
			conn.Read(buffer)
			/*go readResponse(conn, &str1, chDone, chErr)
			select {
			case <-chDone:
				break
			case err = <-chErr:
				break

			}
			*/
		}

		if nil == err {
			i++
			fmt.Printf("%2d !!! %2d *** SUTIJU: \"%s\"\n       Sanemu: \"%s\"\n", reizes, i, str, string(buffer))
			if i >= len(sutit) {
				i = 0
				reizes++

				if reizes >= 2 {
					return
				}
			}

		} else {
			return
		}

		fmt.Println("Osetroviye")
		time.Sleep(1 * time.Second)
		return
	}

	if nil != err {
		fmt.Printf("UDP klienta ERROR: %v\n", err)
	}

}

func readResponse(c net.Conn, msg *string, chDone chan bool, chErr chan error) {

	err := error(nil)
	//	c.SetDeadline(2 * time.Millisecond)
	buffer := make([]byte, 1024)
	buffer = []byte{}

	for {
		_, err = c.Read(buffer)

		if (0 != len(buffer)) || (nil == err) {
			break
		}

		time.Sleep(2 * time.Millisecond)

	}

	if nil == err {
		*msg = string(buffer)
		fmt.Println("!!!", *msg, "!!!")
		chDone <- true
	} else {
		chErr <- err
	}

}

/*
conn, err := net.Dial("udp", "host:port")
if err != nil {
	return err
}
defer conn.Close()

//simple Read
buffer := make([]byte, 1024)
conn.Read(buffer)

//simple write
conn.Write([]byte("Hello from client"))
*/

/*
func main1() {

	vcli.Init()
	nd.cfg()

	fmt.Printf("PATS SAKUMS\n%s\n", nd.String())

	var node vomni.Handler
	chErr := make(chan error)
	chDone := make(chan bool)
	err := error(nil)
	str := ""

	if vomni.ProtocolUDP == nd.Protocol {
		node = udp{node: nd, client: nil, server: nil}
	}

	go node.Connect(chErr)
	err = <-chErr

	if nil == err {
		count := 0
		endIt := false

		stepCount := 3
		step := 1

		for !endIt {
			switch step {
			case 1:
				go node.Waiting(chDone, chErr)
			case 2:
				go node.Receive(&str, chDone, chErr)
			case 3:
				go node.Send(str, chDone, chErr)
			}

			select {
			case err = <-chErr:
				endIt = true
				break
			case <-chDone:
				break
			}

			step++
			if stepCount < step {
				step = 1
			}

			if count >= 20 {
				endIt = true
			}

			count++
			fmt.Printf(" %2d ###############################################\n", count)

		}
	}

	if nil != err {
		fmt.Printf("ERROR: === %v\n", err)
	}

	fmt.Println("FENOFTALEINS ###############################################")

	return

	//str := n1.get()
	//fmt.Println(str)

	fmt.Println(nd.String())
	chEnd := make(chan bool)
	//chErr := make(chan error)

	if vomni.ConnTypeClient == nd.Type {
		go client(nd, "Melioranska", chEnd, chErr)
		select {
		case err := <-chErr:
			fmt.Printf("Client Error: %s\n", err.Error())
			return
		case <-chEnd:
			return
		}
	} else {
		fmt.Println("Serveris vēl nav gatavs")
	}
}
*/

/*
func (node *Node) cfg() {

	//var mi string
	name := vomni.CliTypeName
	node.Type = flag.Lookup(name).Value.String()
	name = vomni.CliProtocolName
	node.Protocol = flag.Lookup(name).Value.String()
	name = vomni.CliIPName
	node.IP = flag.Lookup(name).Value.String()
	name = vomni.CliPortName
	node.Port, _ = strconv.Atoi(flag.Lookup(name).Value.String())
}
*/

func (node *Node) String() (str string) {

	//var mi string
	name := "Type"
	str = fmt.Sprintf("% 25s : %-12s (Type : %s)\n", name, node.Type, reflect.TypeOf(node.Type).Kind())
	name = "Protocol"
	str += fmt.Sprintf("% 25s : %-12s (Type : %s)\n", name, node.Protocol, reflect.TypeOf(node.Protocol).Kind())
	name = "IP"
	str += fmt.Sprintf("% 25s : %-12s (Type : %s)\n", name, node.IP, reflect.TypeOf(node.IP).Kind())
	name = "Port"
	str += fmt.Sprintf("% 25s : %-12d (Type : %s)\n", name, node.Port, reflect.TypeOf(node.Port).Kind())

	//fmt.Println("$$$$$$$$$$$$$$$$$$$$$$$ ", node.Type, reflect.TypeOf(node.Type).Kind())

	//	node.Type = flag.Lookup(vomni.CliTypeName).Value.String()
	return str
}

func client(nd *Node, msg string, chEnd chan bool, chErr chan error) {
	//Connect udp
	addr := nd.IP + ":" + strconv.Itoa(nd.Port)
	conn, err := net.Dial(nd.Protocol, addr)
	if err != nil {
		chErr <- err
		return
	}
	defer conn.Close()

	str := msg
	for {
		buffer := make([]byte, 2048)
		err := make(chan error)

		duration := 2 * time.Millisecond

		done := make(chan bool)

		fmt.Printf("MSG \"%s\"\n", str)

		go clientWrite(conn, str, done, err, duration)
		select {
		case ch1 := <-err:
			chErr <- ch1
			return
		case <-done:

		}

		n := 0
		go clientRead(conn, buffer, &n, done, err, duration)
		select {
		case err1 := <-err:
			chErr <- err1
			return
		case <-done:

		}

		fmt.Println("Sjusjura!")

		//		if err = clientDead(conn); nil != err {
		//			chErr <- err
		//			return
		//		}
		//		n, err = conn.Read(buffer)
		///
		//		fmt.Println("Palas")

		//		if nil != err {
		//
		//			fmt.Println("MIXTURA ", err)
		//
		//			chErr <- err
		//		}

		//		fmt.Println("Dževikom")

		if 0 < n {
			str = string(buffer)
		}

		fmt.Printf("Gricenko \"%s\" --> %d \n", str, len(buffer))

		time.Sleep(5 * time.Millisecond)
		//		chErr <- fmt.Errorf("Tiku līdz šai vietai")
		//		if nil != err {
		//			chErr <- err
		//		}

		chEnd <- true
	}

	/*

	   //simple Read
	   buffer := make([]byte, 1024)
	   conn.Read(buffer)

	   //simple write
	   conn.Write([]byte("Hello from client"))
	*/
}

func clientWrite(c net.Conn, msg string, chDone chan bool, chErr chan error, duration time.Duration) {
	//simple write
	if "" == msg {
		fmt.Printf("***** \"%s\" *****\n", msg)
		chDone <- true

		fmt.Println("#######################")

		return
	} else {

		fmt.Printf("CLIENT WRITE \"%s\" %v\n", msg, "" == msg)
	}
	tick := time.NewTicker(duration)
	chErr1 := make(chan error)

	fmt.Printf("Melponema\n")

	go handleClientWrite(c, msg, chErr1)
	select {
	case <-tick.C:
		chDone <- true
		break
	case err := <-chErr1:
		chErr <- err
		break
	}

	//simple write
	//	if _, err := c.Write([]byte(msg)); nil != err {
	//		chErr <- err
	//		return
	//	}

	<-tick.C
	chDone <- true
}

//n, err = conn.Read(buffer)

func clientRead(c net.Conn, buff []byte, count *int, chDone chan bool, chErr chan error, duration time.Duration) {
	tick := time.NewTicker(duration)

	chBytes := make(chan []byte)
	chErr1 := make(chan error)

	go handleClientRead(c, chBytes, count, chErr1)
	select {
	case <-tick.C:
		chDone <- true
		break
	case buff = <-chBytes:
		break
	case err := <-chErr1:
		chErr <- err
		break
	}
}

func handleClientRead(c net.Conn, chBytes chan []byte, count *int, chErr chan error) {
	for {
		// try to read the data
		data := make([]byte, 4096)
		err := error(nil)
		*count, err = c.Read(data)
		if err != nil {
			// send an error if it's encountered
			chErr <- err
			return
		}
		// send data if we read some.

		if 0 < *count {
			fmt.Println("RECEIVED ", string(data))

		}

		chBytes <- data
	}
}

func handleClientWrite(c net.Conn, msg string, chErr chan error) {
	for {
		// try to read the data
		n, err := c.Write([]byte(msg))

		if 0 < n {
			break
		}

		fmt.Printf("Write %d\n", skaits)
		skaits++

		if err != nil {
			// send an error if it's encountered
			chErr <- err
			return
		}
		// send data if we read some.
	}
}
