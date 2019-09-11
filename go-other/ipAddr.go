package main

/*
URL: https://github.com/mccoyst/myip/blob/master/myip.go
URL: http://changsijay.com/2013/07/28/golang-get-ip-address/
*/

import (
	"fmt"
	"net"
	"os/exec"
)

const (
	cmdIP = "curl -s checkip.dyndns.org | sed -e 's/.*Current IP Address: //' -e 's/<.*$//'"
)

var myIP = ""

func main() {
	ip, err := exec.Command("bash", "-c", cmdIP).Output()
	if err != nil {
		fmt.Println("Failed to execute command: %s", cmdIP)
		return
	}

	locIP := GetLocalIP()

	fmt.Println("*** Mana IP adrese - ārējā ***", string(ip), " *** un iekšējā ***", locIP)
}

// GetLocalIP returns the non loopback local IP of the host
func GetLocalIP() string {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		return ""
	}
	for _, address := range addrs {
		// check the address type and if it is not a loopback the display it
		if ipnet, ok := address.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
			if ipnet.IP.To4() != nil {
				return ipnet.IP.String()
			}
		}
	}
	return ""
}
