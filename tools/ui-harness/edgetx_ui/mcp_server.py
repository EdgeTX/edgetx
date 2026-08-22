import json
import sys
from typing import Any

from .core import HarnessError, HarnessService


TOOLS: dict[str, dict[str, Any]] = {
    "edgetx_build_simulator": {
        "description": "Configure and build a simulator target.",
        "inputSchema": {"type": "object", "properties": {"target": {"type": "string", "default": "tx16s"}}},
    },
    "edgetx_start_simulator": {
        "description": "Start a simulator session with optional storage fixtures.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "target": {"type": "string", "default": "tx16s"},
                "sdcard": {"type": "string"},
                "settings": {"type": "string"},
            },
        },
    },
    "edgetx_stop_simulator": {"description": "Stop the active simulator session.", "inputSchema": {"type": "object"}},
    "edgetx_status": {"description": "Return simulator status and LCD geometry.", "inputSchema": {"type": "object"}},
    "edgetx_press": {
        "description": "Press a named radio key.",
        "inputSchema": {
            "type": "object",
            "properties": {"key": {"type": "string"}, "duration_ms": {"type": "integer", "default": 120}},
            "required": ["key"],
        },
    },
    "edgetx_long_press": {
        "description": "Long press a named radio key.",
        "inputSchema": {
            "type": "object",
            "properties": {"key": {"type": "string"}, "duration_ms": {"type": "integer", "default": 800}},
            "required": ["key"],
        },
    },
    "edgetx_rotate": {
        "description": "Send rotary encoder steps.",
        "inputSchema": {"type": "object", "properties": {"steps": {"type": "integer"}}, "required": ["steps"]},
    },
    "edgetx_touch": {
        "description": "Tap the touch screen.",
        "inputSchema": {
            "type": "object",
            "properties": {
                "x": {"type": "integer"},
                "y": {"type": "integer"},
                "duration_ms": {"type": "integer", "default": 120},
            },
            "required": ["x", "y"],
        },
    },
    "edgetx_wait": {
        "description": "Wait for a number of milliseconds.",
        "inputSchema": {"type": "object", "properties": {"ms": {"type": "integer"}}, "required": ["ms"]},
    },
    "edgetx_screenshot": {
        "description": "Capture a framebuffer screenshot as PNG.",
        "inputSchema": {
            "type": "object",
            "properties": {"name": {"type": "string"}, "out_dir": {"type": "string"}},
            "required": ["name"],
        },
    },
    "edgetx_run_flow": {
        "description": "Run a JSON UI flow.",
        "inputSchema": {"type": "object", "properties": {"flow_path": {"type": "string"}}, "required": ["flow_path"]},
    },
}


class McpServer:
    def __init__(self) -> None:
        self.service = HarnessService()

    def handle(self, request: dict[str, Any]) -> dict[str, Any] | None:
        method = request.get("method")
        request_id = request.get("id")
        try:
            if method == "initialize":
                return self.response(request_id, {"protocolVersion": "2024-11-05", "serverInfo": {"name": "edgetx-ui-harness", "version": "0.1.0"}, "capabilities": {"tools": {}}})
            if method == "notifications/initialized":
                return None
            if method == "tools/list":
                return self.response(request_id, {"tools": [{"name": name, **schema} for name, schema in TOOLS.items()]})
            if method == "tools/call":
                params = request.get("params", {})
                result = self.call_tool(params.get("name"), params.get("arguments", {}))
                return self.response(request_id, {"content": [{"type": "text", "text": json.dumps(result, indent=2)}]})
            return self.error(request_id, -32601, f"unknown method: {method}")
        except Exception as exc:
            return self.error(request_id, -32000, str(exc))

    def call_tool(self, name: str, args: dict[str, Any]) -> dict[str, Any]:
        if name == "edgetx_build_simulator":
            return self.service.build(args.get("target", "tx16s"))
        if name == "edgetx_start_simulator":
            return self.service.start(args.get("target", "tx16s"), args.get("sdcard"), args.get("settings"))
        if name == "edgetx_stop_simulator":
            return self.service.stop()
        if name == "edgetx_status":
            return self.service.status()
        if name == "edgetx_press":
            return self.service.press(args["key"], int(args.get("duration_ms", 120)))
        if name == "edgetx_long_press":
            return self.service.long_press(args["key"], int(args.get("duration_ms", 800)))
        if name == "edgetx_rotate":
            return self.service.rotate(int(args["steps"]))
        if name == "edgetx_touch":
            return self.service.touch(int(args["x"]), int(args["y"]), int(args.get("duration_ms", 120)))
        if name == "edgetx_wait":
            return self.service.wait(int(args["ms"]))
        if name == "edgetx_screenshot":
            return self.service.screenshot(args["name"], args.get("out_dir"))
        if name == "edgetx_run_flow":
            return self.service.run_flow(args["flow_path"])
        raise HarnessError(f"unknown tool: {name}")

    @staticmethod
    def response(request_id: Any, result: dict[str, Any]) -> dict[str, Any]:
        return {"jsonrpc": "2.0", "id": request_id, "result": result}

    @staticmethod
    def error(request_id: Any, code: int, message: str) -> dict[str, Any]:
        return {"jsonrpc": "2.0", "id": request_id, "error": {"code": code, "message": message}}


def main() -> int:
    server = McpServer()
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        response = server.handle(json.loads(line))
        if response is not None:
            print(json.dumps(response), flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
