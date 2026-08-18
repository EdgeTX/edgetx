"""Dependency-free host client for EdgeTX simulator UI automation."""

from .protocol import Event, ProtocolViolation, Response
from .session import (
    CommandFailed,
    ProcessExited,
    ProtocolFailure,
    RequestTimeout,
    SessionError,
    SimulatorSession,
)

__all__ = [
    "CommandFailed",
    "Event",
    "ProcessExited",
    "ProtocolFailure",
    "ProtocolViolation",
    "RequestTimeout",
    "Response",
    "SessionError",
    "SimulatorSession",
]
