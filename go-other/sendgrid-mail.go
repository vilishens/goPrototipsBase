package main

import (
	"fmt"
	"log"
	"os"

	"github.com/sendgrid/sendgrid-go"
	"github.com/sendgrid/sendgrid-go/helpers/mail"
)

func main() {
	from := mail.NewEmail("Automated-Robot", "test@example.com")
	subject := "Sending Test message Raspberry from Pirchilava"
	to := mail.NewEmail("Siskimo", "vilis@mail.com")
	plainTextContent := "and easy to do anywhere, even with Go"
	htmlContent := "<strong>and easy to do anywhere, even with Go</strong>"
	message := mail.NewSingleEmail(from, subject, to, plainTextContent, htmlContent)

	if err := os.Setenv("SENDGRID_API_KEY", "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"); err != nil {
		fmt.Printf("Sakmoto-Yakmoto! ===> nevaru turpināt\n%v\n", err)
		return

	}

	client := sendgrid.NewSendClient(os.Getenv("SENDGRID_API_KEY"))
	response, err := client.Send(message)
	if err != nil {
		log.Println(err)
	} else {
		fmt.Println(response.StatusCode)
		fmt.Println(response.Body)
		fmt.Println(response.Headers)
	}
}
