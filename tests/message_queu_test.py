from queue import Queue
from time import sleep

import pytest
from message_queue import MessageQueue  # type: ignore


@pytest.mark.timeout(15)
def test_message_queue():
    result = Queue[int]()

    def process(value: int) -> None:
        print(f"in python received value {value}")
        sleep(0.01)
        result.put(value)

    queue = MessageQueue(callback=process, size=8)
    queue.start()
    while result.qsize() < 5:
        sleep(1)
        pass

    assert 1 == 1
