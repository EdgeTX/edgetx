"""Dependency-free host client for EdgeTX simulator UI automation."""

from .protocol import (
    CaptureArtifact,
    Event,
    FrameBarrier,
    LuaReload,
    ProtocolViolation,
    Response,
)
from .session import (
    CaptureBundle,
    CommandFailed,
    ProcessExited,
    ProtocolFailure,
    RequestTimeout,
    SessionError,
    SimulatorSession,
)

__all__ = [
    "CaptureArtifact",
    "CaptureBundle",
    "CommandFailed",
    "Event",
    "FrameBarrier",
    "LuaReload",
    "ProcessExited",
    "ProtocolFailure",
    "ProtocolViolation",
    "RequestTimeout",
    "Response",
    "SessionError",
    "SimulatorSession",
]
