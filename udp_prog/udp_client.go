package main

import (
	"fmt"
	"net"
	"strconv"
	"time"
)

func main() {
	//p := make([]byte, 2048)
	conn, err := net.Dial("udp", "127.0.0.1:8000")
	if err != nil {
		fmt.Printf("Some error %v", err)
		return
	}

	pps := 2000
	count := 1
	total := 10000
	ticker := time.NewTicker(1 * time.Second)
	for range ticker.C {
		for i := 0; i < pps; i++ {
			fmt.Fprintf(conn, strconv.FormatInt(int64(count), 10))
			count++
		}

		if count >= total {
			break
		}
	}
	//_, err = bufio.NewReader(conn).Read(p)
	//if err == nil {
	//	fmt.Printf("%s\n", p)
	//} else {
	//	fmt.Printf("Some error %v\n", err)
	//}
	conn.Close()
}

