from queue import Queue
from time import sleep

import pytest
from message_queue import MessageQueue  # type: ignore


@pytest.mark.timeout(15)
def test_message_queue():
    result = Queue[int](1_000_000)

    def process(value: int) -> None:
        result.put(value)

    queue = MessageQueue(callback=process, size=8)
    queue.start()
    while result.qsize() < 900000:
        sleep(0.001)
    del queue

    for i, v in enumerate(result.queue):
        assert i == v
