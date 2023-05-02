import io
import asyncio


# Not present in Adafruit's asyncio, so copied here...
# needs #define MICROPY_PY_IO_IOBASE (1) in mpconfigport.h
class ThreadSafeFlag(io.IOBase):
    def __init__(self):
        self._flag = 0

    def ioctl(self, req, flags):
        if req == 3:  # MP_STREAM_POLL
            return self._flag * flags
        return None

    def set(self):
        self._flag = 1

    async def wait(self):
        if not self._flag:
            await asyncio.core._io_queue.queue_read(self)
        self._flag = 0
