from datetime import datetime
from threading import Thread
from requests import get
from typing import List
from sys import argv

# this script is used to send bunch of requests real fast
# hence the name "pressure"


def request(url: str) -> bool:
    try:
        r = get(url)
        if r.status_code != 200:
            print("received non-OK response (%d)" % r.status_code)
            return False

        return True
    except Exception as e:
        print("request failed: %s" % e)
        return False


def thread(id: int, rc: int, url: str) -> None:
    for _ in range(rc):
        if not request(url):
            print("thread %d failed" % id)
            break


if __name__ == "__main__":
    if len(argv) < 2:
        print("usage: %s <url> [thread count] [request count]" % argv[0])
        exit(1)

    threads: List[Thread] = []
    thread_count = int(argv[2]) if len(argv) >= 3 else 10
    req_count = int(argv[3]) if len(argv) >= 4 else 1000

    if thread_count <= 0:
        print("invalid thread count")
        exit(1)

    if req_count <= 0:
        print("invalid request count")
        exit(1)

    # create all the threads
    for i in range(thread_count):
        print("creating thread %d" % i)
        threads.append(
            Thread(
                target=thread,
                args=(
                    i,
                    req_count,
                    argv[1],
                ),
            )
        )

    start = datetime.now()

    # start all of them
    for t in threads:
        t.start()

    # wait for all of them (will hang)
    for t in threads:
        t.join()

    secs = (datetime.now() - start).total_seconds()
    print(f"took {secs} seconds")
