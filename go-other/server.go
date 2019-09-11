package main

import (
	"fmt"
	"net"
	"strconv"
)

const (
	localIP   = "192.168.1.30" //"127.0.0.1"
	localPort = 49750
)

func sendResponse(conn *net.UDPConn, addr *net.UDPAddr, msg string, chErr chan error) {

	//fmt.Println("IP ", addr, " MSG ", msg)

	str := fmt.Sprintf("Get back: \"%s\" + Sitkovecky", msg)

	_, err := conn.WriteToUDP([]byte(str), addr)

	chErr <- err
}

func main() {
	//	p := make([]byte, 2048)
	addr := net.UDPAddr{
		Port: localPort,
		IP:   net.ParseIP(localIP),
	}

	addrStr := localIP + ":" + strconv.Itoa(localPort)
	//	buff := make([]byte, 2048)
	chErr := make(chan error)

	fmt.Println("IP ", addr.IP, "PORT", addr.Port)

	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		fmt.Printf("Couldn't get connection of %s\n", addrStr)
		return
	}
	for {
		// gaidu UDP

		buff := make([]byte, 2048)
		_, msgAddr, err := conn.ReadFromUDP(buff)
		fmt.Printf("XXX Read a message from %v %s \n", msgAddr, buff)
		if err != nil {
			fmt.Printf("READ ERROR %v", err)
			continue
		}
		go sendResponse(conn, msgAddr, string(buff), chErr)
		err = <-chErr

		if err != nil {
			fmt.Printf("SEND ERROR  %v", err)
			continue
		}
	}
}
