## Library for logging messages with different levels of importance + app that demonstrates its functionality


### Build Instructions
   ```bash
   mkdir build && cd build
   ```

   ```bash
   cmake ..
   ```

   ```bash
   make
   ```
For Google Test:
   ```bash
   ./test_logger
   ```

### Usage

```bash
./logg <filename> <default_level>
```
- E.g. `./logg log.txt INFO`
- `[message] [level]` or  `exit`

```bash
./stats_collector <host> <port> <N> <T>
```
- E.g. `./stats_collector 0.0.0.0 12345 10 60`
- `0.0.0.0` - listens on all interfaces
- `N` - message interval
- `T` - timeout in seconds

```bash
./test_sender <host> <port> <default_level>
```
- E.g. `./test_sender 127.0.0.1 12345 INFO`
- `[message] [level]` or  `exit`

#### Example 1 

1. Start the logging app:
```bash
./build/logg log.txt INFO
```
2. Enter messages like `Test message ERROR` to send logs.

3. Lookup the `log.txt` to check the written logs.

#### Example 2

1. Start the statistics collector:
```bash
./build/stats_collector 0.0.0.0 12345 5 30
```
2. Run the test sender to send logs:
```bash
./build/test_sender 127.0.0.1 12345 INFO
```
3. Enter messages like `Test message ERROR` to send logs and see statistics.
