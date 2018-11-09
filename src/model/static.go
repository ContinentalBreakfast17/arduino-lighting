package model

import "fmt"

type SttcConfig struct {
	Color [3]byte `json:"color"`
	Pins  [3]int  `json:"pins"`
	Mode  int     `json:"mode"`
	Wait  int     `json:"wait"`
}

func (cfg *SttcConfig) Marshal() ([]byte, error) {
	result := fmt.Sprintf(`s{"color":[%d,%d,%d],"pins":[%d,%d,%d],"mode":%d,"speed":%d}`,
		cfg.Color[0], cfg.Color[1], cfg.Color[2], 
		cfg.Pins[0], cfg.Pins[1], cfg.Pins[2], 
		cfg.Mode, cfg.Wait)
	return []byte(result), nil
}