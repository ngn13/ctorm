use tide::Request;
use tide::prelude::*;

#[async_std::main]
async fn main() -> tide::Result<()> {
    let mut app = tide::new();
    app.at("/").get(index);
    app.listen("127.0.0.1:8080").await?;
    Ok(())
}

async fn index(mut req: Request<()>) -> tide::Result {
    Ok(format!("hello world!").into())
}
