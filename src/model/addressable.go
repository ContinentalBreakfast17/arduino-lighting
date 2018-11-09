package model

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"os"
)

type AddrConfig struct {
	Channels []Channel `json:"channels"`
}

type Channel struct {
	Pin      int      `json:"pin"`
	Wait     int      `json:"wait"`
	LedCount int      `json:"ledCount"`
	SeqSize  int      `json:"seqSize"`
	Sequence Sequence `json:"sequence"`
}

type Sequence []Cycle

type Cycle struct {
	Wait     int32     `json:"wait"`
	Strip    [][3]byte `json:"strip"`
}

func (cfg *AddrConfig) Marshal() ([]byte, error) {
	channelStr := "a["
	for _, channel := range cfg.Channels {
		channelStr += fmt.Sprintf(`{"pin":%d,"speed":%d,"ledCount":%d,"seqSize":%d},`, channel.Pin, channel.Wait, channel.LedCount, channel.SeqSize)
	}
	channelStr = channelStr[:len(channelStr)-1] + "]"

	sequence := make([]byte, 0)
	for _, channel := range cfg.Channels {
		for _, cycle := range channel.Sequence {
			for _, led := range cycle.Strip {
				sequence = append(sequence, led[:]...)
			}
			buf := new(bytes.Buffer)
			if err := binary.Write(buf, binary.LittleEndian, cycle.Wait); err != nil {
				return nil, err
			}
			sequence = append(sequence, buf.Bytes()...)
		}
	}
	sequence = append(sequence, 0)
	return append([]byte(channelStr), sequence...), nil
}

func (cfg *AddrConfig) Print() error {
	for channelIndex, channel := range cfg.Channels {
		for cycleIndex, cycle := range channel.Sequence {
			filename := fmt.Sprintf("%03d-%03d", channelIndex, cycleIndex)
			file, err := os.Create("sd/"+filename)
			if err != nil {
				return err
			}
			for _, led := range cycle.Strip {
				n, err := file.Write(led[:])
				if err != nil {
					return err
				} else if n != 3 {
					return fmt.Errorf("bad write %d", n)
				}
			}

			buf := &bytes.Buffer{}
			if err := binary.Write(buf, binary.LittleEndian, cycle.Wait); err != nil {
				return err
			}

			if _, err := file.Write(buf.Bytes()); err != nil {
				return err
			}

			if err := file.Close(); err != nil {
				return err
			}
		}
	}
	return nil
}