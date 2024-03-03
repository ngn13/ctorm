package main

import "github.com/gofiber/fiber/v3"

func main(){
  app := fiber.New()
  app.Get("/", func(c fiber.Ctx) error {
    return c.SendString("hello world!")
  })
  app.Listen("127.0.0.1:8080")
}
