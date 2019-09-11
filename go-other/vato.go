/*
 Example application using Basic auth
 Build with:
 go build basic.go
*/

package main

import (
	"fmt"
	"net/http"

	auth "github.com/abbot/go-http-auth"
)

func handle(w http.ResponseWriter, r *auth.AuthenticatedRequest) {
	fmt.Fprintf(w, "<html><body><h1>Hello, %s!</h1></body></html>", r.Username)
}

func main() {
	authenticator := auth.NewBasicAuthenticator("example.com", Secret)
	http.HandleFunc("/", authenticator.Wrap(handle))
	http.ListenAndServe(":8080", nil)
}
