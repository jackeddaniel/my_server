The build order
Each milestone is something that runs. Resources are attached to the problems you'll hit, not to chapters you should pre-read.

Milestone 1 — "Hello, Browser"
What you build: Server that opens a socket, accepts one connection, sends HTTP/1.1 200 OK\r\n\r\nHello and closes. That's it. Open your browser, see Hello.
You'll immediately hit:

How do I open a socket and bind to a port? → Beej's Guide ch. 1–5 (beej.us/guide/bgnet) — free, 30 mins, just what you need
What is this sockaddr_in struct? → man 2 bind, man 7 ip
Why does bind fail when I restart? → Google SO_REUSEADDR TIME_WAIT, then read TLPI ch. 56 (The Linux Programming Interface) for the real answer

You're done when: curl http://localhost:8080 returns something.

Milestone 2 — Parse a real HTTP request
What you build: Read what the browser actually sends, print it to stdout, parse out the method and path, respond with them echoed back.
You'll immediately hit:

What does a raw HTTP request actually look like byte for byte? → RFC 9112 §2–3 (short, readable, just the message format section) — you need the actual spec here, not a tutorial
How do I read from a socket reliably? → man 2 recv. Learn why you must loop — one recv call does not guarantee you got the whole request
How do I parse headers without losing my mind? → std::string_view for zero-copy parsing. cppreference — string_view. This is the right tool.

You're done when: You can print "GET /about" from whatever the browser sends.

Milestone 3 — Serve a static file
What you build: Map URL paths to files on disk. GET /index.html reads and sends ./static/index.html. Handle 404.
You'll immediately hit:

How do I open and read a file in C++? → man 2 open, man 2 read, man 2 stat. Do it with syscalls, not fopen — you're in systems land now
How do I set Content-Type correctly? → MDN — MIME types for the list, just hardcode a lookup table for 5–6 types
What goes in an HTTP response exactly? → RFC 9110 §15 (status codes) and RFC 9112 §4 (response format). Read these two sections only.
Path traversal — what if someone requests GET /../../etc/passwd? → Think about this. Fix it. This is the kind of thing interviewers ask about.

You're done when: You can serve an HTML file with an image and a CSS file, and the browser renders it correctly.

Milestone 4 — Handle multiple connections
What you build: Stop handling one request and dying. Keep the server alive across many requests.
You'll immediately hit:

The simplest fix — a while(true) accept loop — blocks on one connection at a time. What's wrong with that? → This is the moment to read "The C10K Problem" by Dan Kegel (kegel.com/c10k.html). Written in 2002, still the canonical framing of why this matters.
Thread per connection: try it, feel it work, then read why it doesn't scale → UNP ch. 26 (Unix Network Programming by Stevens) — threads intro, one chapter
What's the right model? → Now you're ready to understand epoll. man 7 epoll — read the whole thing, it's not long. Then LWN "Epoll is fundamentally broken" (lwn.net, 2017) for why even epoll has sharp edges.

You're done when: Your server handles 10 simultaneous curl requests without hanging.

Milestone 5 — epoll event loop
What you build: Rip out your blocking accept loop. Replace with a proper epoll-based non-blocking event loop. This is the architectural heart of the server.
You'll immediately hit:

Edge-triggered vs level-triggered — you will get this wrong the first time → TLPI ch. 63 is the definitive explanation. Read it when your server starts dropping connections mysteriously.
Non-blocking sockets and EAGAIN — recv returning -1 doesn't always mean error → man 2 recv, the ERRORS section specifically
Managing connection state — each fd now needs to remember where it is in parsing → design a Connection struct. Think about this before you code it. nginx's ngx_connection_t is worth glancing at for inspiration (src/core/ngx_connection.h on GitHub)
SIGPIPE — your server will crash silently when a client disconnects mid-write → man 7 signal, then signal(SIGPIPE, SIG_IGN). One line fix, important to know why.

You're done when: Your event loop handles new connections, readable data, and disconnects without blocking, and doesn't crash when clients misbehave.

Milestone 6 — Thread pool
What you build: epoll detects ready connections, hands them off to a pool of worker threads. The event loop never does real work itself.
You'll immediately hit:

How do I build a work queue that multiple threads share safely? → "C++ Concurrency in Action" ch. 6 (Anthony Williams) — specifically the thread-safe queue section. This is the exact data structure you need.
How many threads? → Read about the difference between I/O-bound and CPU-bound work. For a web server, number of cores × 1–2 is a good starting point. Know why.
Condition variables — your threads need to sleep when there's no work → cppreference — std::condition_variable. The wait with predicate form. Don't use the version without a predicate, it has spurious wakeup bugs.

You're done when: You have N worker threads, a shared queue, and the epoll loop is just pushing fds onto it.

Milestone 7 — Keep-alive + pipelining
What you build: Support Connection: keep-alive. Reuse connections across multiple requests instead of closing after each one.
You'll immediately hit:

Your state machine gets more complex — a connection can now be in "reading request 1", "sending response 1", "reading request 2" → think carefully about your state enum before touching code
How do you know when a request ends and the next begins? → Content-Length and Transfer-Encoding: chunked — RFC 9112 §6 explains message framing. This is the section that matters.
Timeouts — idle keep-alive connections should close after N seconds → man 2 timerfd_create for the right way to do this with epoll

You're done when: A single curl --keepalive session sends multiple requests over one connection successfully.

Milestone 8 — Benchmark and document
What you build: Nothing new. You run wrk against your server and write a README that explains every decision you made.
You'll immediately hit:

wrk -t4 -c100 -d10s http://localhost:8080/ — what do the numbers mean, and how do you improve them?
Profiling: perf stat, perf record + perf report → Brendan Gregg's blog (brendangregg.com) — "Linux Performance" page. Learning to read a flamegraph here will pay dividends forever.
Your README should answer: architecture decisions, concurrency model, benchmarks, known limitations, what you'd do next → look at libuv's README and nginx's architecture doc (aosabook.org — free) as examples of how good systems projects explain themselves

You're done when: You have benchmark numbers, a clear README, and you can explain every decision out loud without looking at the code.

The three books to have open the whole time
These aren't things to read cover to cover. They're references you'll reach for constantly:

TLPI (The Linux Programming Interface, Kerrisk) — the answer to almost every syscall question
UNP (Unix Network Programming, Stevens) — the answer to almost every socket/TCP question
Beej's Guide (free, beej.us) — fastest way to get unstuck on socket API stuff


Timeline in this mode
MilestoneRealistic time if you're hacking daily1–2Weekend 13Week 14Week 25 (epoll)Weeks 3–4 — this is the hard one6 (thread pool)Week 57 (keep-alive)Week 68 (benchmark)Week 7
The epoll milestone will take longer than you think. That's normal and fine — everyone rewrites their event loop at least twice.
