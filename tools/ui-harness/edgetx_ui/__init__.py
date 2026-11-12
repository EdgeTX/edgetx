"""Dependency-free host client for EdgeTX simulator UI automation."""

from .flow import (
    FlowDefinition,
    FlowError,
    FlowExecutionError,
    FlowRunResult,
    FlowRunner,
    FlowValidationError,
    load_flow,
)
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
    "FlowDefinition",
    "FlowError",
    "FlowExecutionError",
    "FlowRunResult",
    "FlowRunner",
    "FlowValidationError",
    "FrameBarrier",
    "LuaReload",
    "ProcessExited",
    "ProtocolFailure",
    "ProtocolViolation",
    "RequestTimeout",
    "Response",
    "SessionError",
    "SimulatorSession",
    "load_flow",
]
