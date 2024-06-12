use tide::Request;
use tide::prelude::*;

#[async_std::main]
async fn main() -> tide::Result<()> {
    // create the app
    let mut app = tide::new();

    // setup the "hello world!" route
    app.at("/").get(index);

    // start the app
    app.listen("127.0.0.1:8080").await?;
    Ok(())
}

async fn index(mut req: Request<()>) -> tide::Result {
    Ok(format!("hello world!").into())
}
