import sys
from typing import Any, Dict, Iterator, KeysView, Optional, ValuesView, ItemsView


class LoggingDict:
    """A dictionary wrapper that logs all lookups to stderr."""

    def __init__(self, data: Optional[Dict[Any, Any]] = None):
        self._data = data if data is not None else {}

    def __getitem__(self, key: Any) -> Any:
        """Log the lookup and return the value."""
        print(f"{str(key)}", file=sys.stderr)
        return self._data[key]

    def __setitem__(self, key: Any, value: Any) -> None:
        """Set an item in the dictionary."""
        self._data[key] = value

    def __delitem__(self, key: Any) -> None:
        """Delete an item from the dictionary."""
        del self._data[key]

    def __contains__(self, key: Any) -> bool:
        """Check if key exists (this is also a lookup, so log it)."""
        print(f"{str(key)}", file=sys.stderr)
        return key in self._data

    def __len__(self) -> int:
        """Return the length of the dictionary."""
        return len(self._data)

    def __iter__(self) -> Iterator:
        """Return an iterator over the keys."""
        return iter(self._data)

    def __repr__(self) -> str:
        """Return a string representation."""
        return f"LoggingDict({self._data})"

    def get(self, key: Any, default: Any = None) -> Any:
        """Get a value with optional default (logs the lookup)."""
        print(f"{str(key)}", file=sys.stderr)
        return self._data.get(key, default)

    def keys(self) -> KeysView:
        """Return dictionary keys."""
        return self._data.keys()

    def values(self) -> ValuesView:
        """Return dictionary values."""
        return self._data.values()

    def items(self) -> ItemsView:
        """Return dictionary items."""
        return self._data.items()

    def update(self, other: Dict[Any, Any]) -> None:
        """Update the dictionary with another dictionary."""
        self._data.update(other)

    def clear(self) -> None:
        """Clear all items from the dictionary."""
        self._data.clear()

    def pop(self, key: Any, default: Any = None) -> Any:
        """Remove and return a value (logs the lookup)."""
        print(f"{str(key)}", file=sys.stderr)
        if default is None:
            return self._data.pop(key)
        return self._data.pop(key, default)

    def setdefault(self, key: Any, default: Any = None) -> Any:
        """Get a value or set/return default (logs the lookup)."""
        print(f"{str(key)}", file=sys.stderr)
        return self._data.setdefault(key, default)


# Example usage
if __name__ == "__main__":
    # Create a logging dictionary
    d = LoggingDict({"a": 1, "b": 2, "c": 3})

    # These operations will be logged to stderr
    print("Value of 'a':", d["a"])
    print("Value of 'b':", d.get("b"))
    print("Does 'c' exist?", "c" in d)
    print("Does 'd' exist?", "d" in d)

    # Add new items
    d["d"] = 4
    d["e"] = 5

    # More logged lookups
    print("Value of 'd':", d["d"])
    print("Popped 'e':", d.pop("e"))

    print("Final dictionary:", d)
