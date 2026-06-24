# my_server

A small HTTP/1.1 web server written from scratch in C++ on Linux, using raw
sockets and the `epoll` event loop — no web framework, no libraries beyond the
standard library and the Linux syscall interface.

> ⚠️ **This is a learning project.** It exists to understand how web servers
> actually work under the hood — sockets, the HTTP wire format, non-blocking
> I/O, and the different concurrency models the kernel gives you. It is **not**
> production software: there's no TLS, no request size limits, no robust error
> recovery, and only a tiny slice of HTTP is implemented. Don't deploy it.

## What it does

- Opens a TCP listening socket and binds to port `3490`
- Parses incoming HTTP/1.1 requests (method, path, headers, and `Content-Length` body framing)
- Serves static files from `./static/` with correct `Content-Type` inference
- Returns `404` for paths that don't resolve, with path-traversal protection
- Handles many simultaneous connections through a single-threaded, non-blocking `epoll` event loop

## Learning roadmap

The project follows the milestone plan in [`agenda.md`](agenda.md), building the
server up one capability at a time:

| Milestone | Goal | Status |
|-----------|------|--------|
| 1 | Hello, Browser — open a socket, send one response | ✅ done |
| 2 | Parse a real HTTP request | ✅ done |
| 3 | Serve a static file (with MIME types + 404) | ✅ done |
| 4 | Handle multiple connections | ✅ done |
| 5 | `epoll` non-blocking event loop | ✅ done (current) |
| 6 | Thread pool | ⬜ next |
| 7 | Keep-alive + pipelining | ⬜ |
| 8 | Benchmark and document | ⬜ |

Each architecture was built on its own branch so the different concurrency
models can be compared side by side.

## Branches

This repo deliberately keeps each concurrency model on a separate branch so the
trade-offs are easy to compare:

| Branch | Concurrency model | Notes |
|--------|-------------------|-------|
| `main` | baseline | starting point |
| `fork_server` | one `fork()` per connection | simplest, but a new process per client doesn't scale |
| `poll_server` | single-threaded `poll()` event loop | one thread multiplexes all connections |
| `epoll_server` | single-threaded `epoll` event loop | **current branch** — the scalable Linux approach |

## Architecture (epoll_server)

A single thread runs an `epoll_wait` loop ([`server.cpp`](server.cpp)):

- The listening socket and all client sockets are non-blocking and registered with `epoll`.
- New connections are accepted in a loop until `EAGAIN` (`handle_new_connections`).
- Per-connection state lives in a `conn_state` struct ([`http.h`](http.h)) keyed by fd — receive buffer, parse offsets, send buffer/offset, and a state enum (`READING` → `SENDING`).
- A connection is watched for `EPOLLIN` while reading the request, then switched to `EPOLLOUT` (`EPOLL_CTL_MOD`) once a full response is queued to send.
- `SIGPIPE` is ignored so a client disconnecting mid-write doesn't kill the server.

### Files

| File | Responsibility |
|------|----------------|
| `server.cpp` | socket setup, epoll loop, connection state machine |
| `http.h` | shared structs (`conn_state`, `http_request`, `http_response`) and declarations |
| `http_helpers.cpp` | request parsing and response building |
| `file_helpers.cpp` | path resolution, file reading, MIME-type inference |
| `static/` | files served to clients (`index.html`, `style.css`, `app.js`, …) |
| `testing/` | stress test script and partial-recv test |
| `experiments/` | scratch socket programs written while learning the APIs |

## Build & run

There's no build system — just compile the server and its helpers directly:

```sh
g++ -std=c++17 server.cpp http_helpers.cpp file_helpers.cpp -o server.out
./server.out
```

Then open <http://localhost:3490/index.html> in a browser, or:

```sh
curl http://localhost:3490/test.html
```

## Benchmarks

Benchmarked with `wrk -t4 -c100 -d30s http://localhost:3490/test.html`. Full
results, including a scaling sweep (10 → 1000 connections) and comparisons
against nginx and FastAPI, are in [`web_server_testing.md`](web_server_testing.md).

| Server | Requests/sec (`-c100`) |
|--------|------------------------|
| fork | ~9,400 |
| poll | ~31,800 |
| epoll | ~33,300 |
| nginx (reference) | ~163,000 |
| FastAPI (reference) | ~2,400 |

The progression from `fork` → `poll` → `epoll` is the whole point: each step
shows measurably why the next concurrency model matters.

## Known limitations

- Single-threaded — no thread pool yet (milestone 6)
- Closes the connection after each response — no keep-alive yet (milestone 7)
- Only `GET`-style static file serving; no dynamic routing
- No TLS, no request body size limits, minimal error handling
