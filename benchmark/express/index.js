const express = require("express")
const app = express()

app.get("/", (_, res) => {
  res.send("hello world!")
})

app.listen(8080)
