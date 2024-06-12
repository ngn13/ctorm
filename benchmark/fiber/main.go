package main

import "github.com/gofiber/fiber/v3"

func main(){
  // create the app
  app := fiber.New()

  // setup the "hello world!" route
  app.Get("/", func(c fiber.Ctx) error {
    return c.SendString("hello world!")
  })

  // start the app
  app.Listen("127.0.0.1:8080")
}
