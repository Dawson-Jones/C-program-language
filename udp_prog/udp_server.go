package main

import (
	"fmt"
	"log"
	"net"
	"strconv"
)

func sendResponse(conn *net.UDPConn, addr *net.UDPAddr) {
	_, err := conn.WriteToUDP([]byte("From server: Hello I got your message "), addr)
	if err != nil {
		fmt.Printf("Couldn't send response %v", err)
	}
}

func main() {
	p := make([]byte, 2048)
	addr := net.UDPAddr{
		Port: 8000,
		IP:   net.ParseIP("0.0.0.0"),
	}

	conn, err := net.ListenUDP("udp", &addr)
	if err != nil {
		fmt.Printf("Some error %v\n", err)
		return
	}

	b := make([]int, 10001, 10001)
	for {
		_, _, err := conn.ReadFromUDP(p)
		if err != nil {
			fmt.Printf("Some error  %v", err)
			continue
		}
		fmt.Printf("recv %s\n", p)

		var i int
		var v byte
		for i, v = range p {
			if v == 0 {
				break
			}
		}

		c, e := strconv.Atoi(string(p[:i]))
		if err != nil {
			log.Fatalln(e)
		}

		fmt.Printf("convert c: %d\n", c)
		b[c] = 1

		if c == 10000 {
			break
		}
		//go sendResponse(conn, remoteaddr)
	}

	for i, v := range b {
		if v != 1 {
			fmt.Printf("%dth loss, value: %d", i, v)
		}
	}
}

