package main

//go:generate go-bindata -o assets.go assets/vue/... assets/styles.css

import  (
	"encoding/json"
	"errors"
	"io/ioutil"
	"log"
	"net/url"

	"github.com/continentalbreakfast17/src/controller"
	"github.com/zserge/webview"
)

func handle(w webview.WebView, msg string) {
	log.Print(msg)
	
}

func main() {
	// Read html
	html, err := ioutil.ReadFile("assets/web.html")
	errorHandler("Failed to read web.html", err, false)

	// Intialize Webview
	w := webview.New(webview.Settings{
		Title: "Arduino RGB Controller",
		URL: `data:text/html,` + url.PathEscape(string(html)),
		ExternalInvokeCallback: handle,
		Debug: true,
		Resizable: true,
		Width: 800,
		Height: 600,
	})
	defer w.Exit()

	// Dispatch Webview
	w.Dispatch(func() {
		// Inject CSS
		w.InjectCSS(string(MustAsset("assets/styles.css")))

		// Inject Vue.js
		w.Eval(string(MustAsset("assets/vue/vendor/vue.min.js")))
		// Inject app code
		w.Eval(string(MustAsset("assets/vue/app.js")))
	})
	w.Run()
	
}

func errorHandler(msg string, err error, shouldSave bool) {
	if err != nil {
		arduino.Disconnect()
		log.Printf("%s", msg)
		if shouldSave {
			profiles.save()
		}
		panic(err)
	}
}