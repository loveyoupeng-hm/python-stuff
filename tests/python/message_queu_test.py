from queue import Queue
from time import sleep, perf_counter_ns

import pytest
from message_queue import MessageQueue  # type: ignore


@pytest.mark.timeout(15)
def test_message_queue():
    result = Queue[int](200_000)

    def process(value: int) -> None:
        result.put(value)

    queue = MessageQueue(callback=process, size=8)
    queue.start()
    start = perf_counter_ns()
    while result.qsize() < 10_000:
        sleep(0.001)
    end = perf_counter_ns()
    del queue

    print(
        f"perf : {result.qsize()} / {float((end - start)) / 1_000_000_000} {float(end - start) / result.qsize()}"
    )

    for i in range(0, result.qsize()):
        assert i == result.get()
    print("finished")
