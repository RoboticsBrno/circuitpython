import asyncio

from .threadsafeflag import ThreadSafeFlag

# Default is speed at 25C, 50%, 101 kPa
SPEED_OF_SOUND = 347.13 / 100
ULTRASOUND_COUNT = 4


class Ultrasound:
    def __init__(self, idx: int, man_native) -> None:
        self._idx = idx
        self._man_native = man_native
        self._measuring = False
        self._done_ev = ThreadSafeFlag()
        self._last_measured_mm = 0

    def _on_measuring_done(self, roundtrip_us: int) -> None:
        self._last_measured_mm = int((SPEED_OF_SOUND * (roundtrip_us / 10.0)) / 2)
        self._done_ev.set()

    async def measure_mm(self) -> "int | None":
        if not self._measuring:
            self._measuring = True
            self._man_native.sendUtsPingRequest(self._idx)

        try:
            await asyncio.wait_for(self._done_ev.wait(), 0.500)
            return self._last_measured_mm
        except asyncio.TimeoutError:
            return None
        finally:
            self._measuring = False
