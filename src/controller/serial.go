package controller

import (
	"fmt"
	"time"
	"github.com/mikepb/go-serial"
)

type Controller interface {
	Send(interface{}) (byte, error)
	Shutdown() error
}

type Sendable interface {
	Marshal() ([]byte, error)
}

type Serial struct {
	port *serial.Port
}

func NewSerial(port string, baud int) (*Serial, error) {
	//serial.SetDebug(true)
	options := serial.RawOptions
	options.BitRate = baud
	options.Mode = serial.MODE_READ_WRITE

	p, err := options.Open(port)
	if err != nil {
		return nil, fmt.Errorf("Failed to open port: %v", err)
	}

	controller := Serial{p}
	if err := controller.init(); err != nil {
		return nil, fmt.Errorf("Failed to get ready response: %v", err)
	}

	return &controller, nil
}

func (srl *Serial) init() error {
	resp, err := srl.read()
	if err != nil {
		return err
	} else if resp != 0 {
		return fmt.Errorf("Bad response: %d", resp)
	}

	if err := srl.port.Reset(); err != nil {
		return err
	}

	return nil
}

func (srl *Serial) Shutdown() error {
	return srl.port.Close()
}

func (srl *Serial) Send(object Sendable) (byte, error) {
	if err := srl.write(object); err != nil {
		return 0, fmt.Errorf("Failed to write object: %v", err)
	}

	response, err := srl.read()
	if err != nil {
		return 0, fmt.Errorf("Failed to read response: %v", err)
	}

	return response, err
}

func (srl *Serial) write(object Sendable) error {
	message, err := object.Marshal()
	if err != nil {
		return fmt.Errorf("Failed to marshal object: %v", err)
	}
	fmt.Printf("%s\n", message)

	for start := 0; start < len(message); start += 32 {
		end := start + 32
		if end >= len(message) {
			end = len(message) - 1
		}

		retries := 0
		for {
			n, err := srl.port.OutputWaiting()
			if err != nil {
				return fmt.Errorf("Failed to get output waiting: %v", err)
			} else if retries > 5 {
				return fmt.Errorf("Timeout- %d bytes still waiting in output buffer", n)
			} else if n == 0 {
				break
			}

			retries++
			time.Sleep(200 * time.Millisecond)
		}

		buf := message[start:end]
		if _, err := srl.port.Write(buf); err != nil {
			return fmt.Errorf("Failed to write: %v", err)
		}

	}
	return nil
}

func (srl *Serial) read() (byte, error) {
	buf := make([]byte, 1)
	if _, err := srl.port.Read(buf); err != nil {
		return 0, err
	}

	return buf[0], nil
}