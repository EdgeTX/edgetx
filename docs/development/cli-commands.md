# CLI Commands

Summary of the commands available in CLI mode.

## Always available commands

| Function | Parameters (if any) | Description |
|--|--|--|
| `beep` |`[<frequency>] [<duration>]`| Play a beep sound. |
| `ls` | `<directory>`| List files in a directory. |
| `read` |`<filename>` | Read a file. |
| `readsd` |`<start sector> <sectors count> <read buffer size (sectors)>`| Read sectors from SD card. |
| `testsd` |  | Test SD card. |
| `play` | `<filename>` | Play a file. |
| `reboot` | `[wdt]` | Reboot the device. |
| `set` | `<what> <value>` | Set a configuration parameter. |
| `help` | `[<command>]` | Show help for a command. |

## If `ENABLE_SERIAL_PASSTHROUGH` set

| Function | Parameters (if any) | Description |
|--|--|--|
| `serialpassthrough` | `<port type> <port number>` | Enable serial passthrough. |


## If `DEBUG` set

| Function | Parameters (if any) | Description |
|--|--|--|
| `print` | `<address> [<size>] \| <what>` | Print a value. |
| `p` | `<address> [<size>] \| <what>` | Print a value. |
| `stackinfo` | | Show stack info. |
| `meminfo` | | Show memory info. |
| `test` | `new \| graphics \| memspd` | Run a test. |
| `trace` | `on \| off` | Enable/disable tracing. |
| `debugvars` | | Show debug variables. |
| `repeat` | `<interval> <command>` | Repeat a command. |


## If `JITTER_MEASURE` set

| Function | Parameters (if any) | Description |
|--|--|--|
| `jitter` | | Measure jitter. |


## If `INTERNAL_GPS` set

| Function | Parameters (if any) | Description |
|--|--|--|
| `gps` | `<baudrate>\|$<command>\|trace` | Send a command to the GPS. |


## If `BLUETOOTH` set

| Function | Parameters (if any) | Description |
|--|--|--|
| `bt` | `<baudrate>\|<command>` | Send a command to the Bluetooth module. |


## If `ACCESS_DENIED` && `DEBUG_CRYPT` set

| Function | Parameters (if any) | Description |
|--|--|--|
| `crypt` | `<string to be encrypted>` | Encrypt a string. |
