import rbcx_native
import asyncio
import supervisor

from .ultrasound import Ultrasound, ULTRASOUND_COUNT
import sys
import io

from .threadsafeflag import ThreadSafeFlag


class Manager:
    def __init__(self, rx_pin: int, tx_pin: int) -> None:
        self._native = rbcx_native.Manager(self, rx_pin, tx_pin)

        self._version_ev = ThreadSafeFlag()
        self._version_data: "tuple[str, int, bool] | None" = None

        self._ultrasounds: "list[Ultrasound | None]" = [None] * ULTRASOUND_COUNT

    def _on_coproc_versionStat(
        self, version_str: str, version_num: int, version_dirty: bool
    ) -> None:
        ver_data = (version_str, version_num, version_dirty)

        self._version_data = ver_data
        self._version_ev.set()

    def _on_coproc_ultrasoundStat(self, uts_index: int, roundtrip_us: int) -> None:
        if uts_index < 0 or uts_index >= ULTRASOUND_COUNT:
            print("Invalid UTS index %d in _on_coproc_ultrasoundStat" % uts_index, file=sys.stderr)
            return

        uts = self._ultrasounds[uts_index]
        if uts is None:
            print(
                "UTS %d not initialized in _on_coproc_ultrasoundStat" % uts_index, file=sys.stderr
            )
            return

        uts._on_measuring_done(roundtrip_us)

    def send_version_request(self) -> None:
        self._version_data = None
        self._native.sendVersionRequest()

    async def get_version(self) -> "tuple[str, int, bool]":
        self.send_version_request()

        # There must be wait_for here, because if no other coroutines than the .wait()
        # are active, the asyncio loop thinks nothing can set the Event,
        # and it terminates the event loop
        await asyncio.wait_for(self._version_ev.wait(), timeout=5)

        assert self._version_data is not None
        return self._version_data

    def ultrasound(self, uts_index: int) -> Ultrasound:
        if uts_index < 0 or uts_index >= ULTRASOUND_COUNT:
            raise ValueError("UTS index must be in range <0;3> (got %d)" % uts_index)

        instance = self._ultrasounds[uts_index]
        if instance is None:
            instance = Ultrasound(uts_index, self._native)
            self._ultrasounds[uts_index] = instance
        return instance
