from queue import Queue
import threading
from time import sleep

import pytest
from message_queue import MessageQueue  # type: ignore


@pytest.mark.timeout(15)
def test_message_queue():
    result = Queue[int](1_000_000)

    def process(value: int) -> None:
        if value < 100:
            print(f"{threading.get_ident()} received {value=}")
        result.put(value)

    queue = MessageQueue(callback=process, size=8)
    queue.start()
    while result.qsize() < 10:
        sleep(0.001)

    for i, v in enumerate(result.queue):
        assert i == v
    del queue
