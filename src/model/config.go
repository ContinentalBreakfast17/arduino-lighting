package model

type Config struct {
	Sttc SttcConfig `json:"static"`
	Addr AddrConfig `json:"addressable"`
}