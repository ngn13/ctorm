// import express
const express = require("express")
// create the app
const app = express()

// setup the "hello world!" route
app.get("/", (_, res) => {
  res.send("hello world!")
})

// start the app
app.listen(8080)
