# Benchmarks

Currently implemented benchmarks compare *RESTinio* with
with different timers implementatios:

* using asio timers;
* using sobjectizer timers;
* using no timers.

Benchmarks located [here](./dev/bench).

Test were executes on single i7-6700K CPU @ 4.00GHz machine with 16 Gb of RAM
running Ubuntu 16.04.2 LTS.

[wrk](https://github.com/wg/wrk) was used as benchmarking tool.
Running command:
~~~~~
::bash
wrk -t 1 -c 100 -d 10 http://127.0.0.1:8080/
~~~~~

All benchmarks had the same config file:
~~~~~
::JSON
{
	"port" : 8080,
	"protocol" : "ipv4",
	"handle_request_timeout_ms": 1000,
	"buffer_size" : 4096,
	"read_next_http_message_timelimit_ms": 15000,
	"write_http_response_timelimit_ms": 1000
}
~~~~~

Running command:
~~~~~
::bash
_bench.single_handler_so5_timers -c bench.cfg.json --asio-pool-size N
~~~~~

And the size of asio pool `N`: 1, 2, 4.

And results are the following:

| Bench                  | Avg Requests/sec (5 runs)                     |
|------------------------|---------------| --------------|---------------|
| Number of asio threads | 1             | 2             | 4             |
| asio timers            | 121022.51     | 164240.15     | 227143.48     |
| sobjectizer timers     | 124259.11     | 183522.02     | 273669.63     |
| no timers              | 159565.24     | 257555.52     | 397218.99     |
