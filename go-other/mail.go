// using SendGrid's Go Library
// https://github.com/sendgrid/sendgrid-go
package main

import (
	"fmt"
	"log"

	"github.com/sendgrid/sendgrid-go"
	"github.com/sendgrid/sendgrid-go/helpers/mail"
)

func main() {
	from := mail.NewEmail("Example User", "arduino-raspis@mail.com")
	subject := "Sending with SendGrid is Fun"
	//	to := mail.NewEmail("Example User", "test@example.com")
	to := mail.NewEmail("Example User", "vilis@mail.com")
	plainTextContent := "and easy to do anywhere, even with Go"
	htmlContent := "<strong>and easy to do anywhere, even with Go</strong>"
	message := mail.NewSingleEmail(from, subject, to, plainTextContent, htmlContent)
	client := sendgrid.NewSendClient("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")
	response, err := client.Send(message)
	if err != nil {
		fmt.Println("*** ERROR ***")
		log.Println(err)
	} else {
		fmt.Println("*** SUCCESS ***")
		fmt.Println(response.StatusCode)
		fmt.Println(response.Body)
		fmt.Println(response.Headers)
	}
}
