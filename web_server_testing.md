Web Server Testing:

Primary benchmark: wrk \-t4 \-c100 \-d30s [http://localhost:3490/test.html](http://localhost:3490/test.html)

Fork\_server:  
Command \- wrk \-t4 \-c100 \-d30s [http://localhost:3490/test.html](http://localhost:3490/test.html)  
Results \-   
wrk \-t4 \-c100 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency     8.83ms   65.68ms   1.67s    97.41%  
    Req/Sec     2.40k     1.26k    8.05k    66.33%  
  280708 requests in 30.02s, 180.43MB read  
  Socket errors: connect 0, read 0, write 0, timeout 19  
Requests/sec:   9350.91

Transfer/sec:      6.01MB

Poll\_server:  
Command \- wrk \-t4 \-c100 \-d30s [http://localhost:3490/test.html](http://localhost:3490/test.html)  
Results \-   
wrk \-t4 \-c100 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    13.54ms   85.82ms   1.67s    96.33%  
    Req/Sec     8.06k     3.74k   20.26k    68.46%  
  954892 requests in 30.00s, 613.78MB read  
  Socket errors: connect 0, read 0, write 0, timeout 15  
Requests/sec:  31828.82  
Transfer/sec:     20.46MB  
   
Epoll\_server:  
Command \- wrk \-t4 \-c100 \-d30s [http://localhost:3490/test.html](http://localhost:3490/test.html)  
Results \-   
wrk \-t4 \-c100 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    10.48ms   67.04ms   1.66s    96.83%  
    Req/Sec     8.36k     2.69k   20.31k    68.58%  
  998066 requests in 30.00s, 641.53MB read  
  Socket errors: connect 0, read 0, write 0, timeout 1  
Requests/sec:  33264.01  
Transfer/sec:     21.38MB

Scaling test for three architectures \-   
Fork\_server:  
siddhant  \~  echo fork\_10  
fork\_10  
 siddhant  \~  wrk \-t2 \-c10 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  2 threads and 10 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency     1.00ms  395.64us   6.65ms   77.85%  
    Req/Sec     4.63k     0.92k    5.73k    65.83%  
  276654 requests in 30.00s, 177.83MB read  
Requests/sec:   9220.58  
Transfer/sec:      5.93MB  
 siddhant  \~  echo fork\_100  
fork\_100  
 siddhant  \~  wrk \-t4 \-c100 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency     9.29ms   74.51ms   1.67s    97.82%  
    Req/Sec     2.26k     1.25k    7.38k    67.32%  
  264759 requests in 30.03s, 170.18MB read  
  Socket errors: connect 0, read 0, write 0, timeout 9  
Requests/sec:   8816.24  
Transfer/sec:      5.67MB  
 siddhant  \~  echo fork\_500  
fork\_500  
 siddhant  \~  wrk \-t8 \-c500 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  8 threads and 500 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    12.80ms   82.88ms   1.67s    97.14%  
    Req/Sec     1.07k   765.57     5.52k    71.89%  
  242792 requests in 30.08s, 156.06MB read  
  Socket errors: connect 0, read 0, write 0, timeout 79  
Requests/sec:   8071.53  
Transfer/sec:      5.19MB  
 siddhant  \~  127  echo fork\_1000  
fork\_1000  
 siddhant  \~  wrk \-t8 \-c1000 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  8 threads and 1000 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    16.01ms  102.23ms   1.73s    96.89%  
    Req/Sec     1.05k   755.49     4.38k    69.76%  
  243434 requests in 30.08s, 156.47MB read  
  Socket errors: connect 0, read 0, write 0, timeout 109  
Requests/sec:   8092.75  
Transfer/sec:      5.20MB

Poll\_server:  
siddhant  \~  echo poll\_10  
poll\_10  
 siddhant  \~  wrk \-t2 \-c10 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  2 threads and 10 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency   269.44us  111.72us   2.60ms   80.59%  
    Req/Sec    15.69k     2.71k   19.69k    65.22%  
  938096 requests in 30.10s, 602.99MB read  
Requests/sec:  31166.30  
Transfer/sec:     20.03MB  
 siddhant  \~  echo poll\_100  
poll\_100  
 siddhant  \~  wrk \-t4 \-c100 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    12.31ms   76.60ms   1.66s    96.31%  
    Req/Sec     6.76k     3.37k   20.26k    70.48%  
  800981 requests in 30.01s, 514.85MB read  
  Socket errors: connect 0, read 0, write 0, timeout 22  
Requests/sec:  26690.33  
Transfer/sec:     17.16MB  
 siddhant  \~  echo poll\_500  
poll\_500  
 siddhant  \~  wrk \-t8 \-c500 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  8 threads and 500 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    15.77ms  106.75ms   1.92s    96.94%  
    Req/Sec     3.99k     2.58k   15.25k    69.35%  
  900504 requests in 30.01s, 578.82MB read  
  Socket errors: connect 0, read 0, write 0, timeout 66  
Requests/sec:  30004.61  
Transfer/sec:     19.29MB  
 siddhant  \~  echo poll\_1000  
poll\_1000  
 siddhant  \~  wrk \-t8 \-c1000 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  8 threads and 1000 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    13.77ms  100.92ms   1.92s    97.45%  
    Req/Sec     3.90k     2.56k   21.43k    69.23%  
  890931 requests in 30.01s, 572.67MB read  
  Socket errors: connect 0, read 17, write 0, timeout 169  
Requests/sec:  29691.25  
Transfer/sec:     19.08MB

Epoll\_server:  
 siddhant  \~  echo epoll\_10  
epoll\_10  
 siddhant  \~  wrk \-t2 \-c10 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  2 threads and 10 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency   214.69us   59.65us   1.73ms   70.21%  
    Req/Sec    19.23k     4.04k   25.02k    61.96%  
  1151838 requests in 30.10s, 740.37MB read  
Requests/sec:  38267.62  
Transfer/sec:     24.60MB  
 siddhant  \~  echo epoll\_100  
epoll\_100  
 siddhant  \~  wrk \-t4 \-c100 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency     2.46ms  648.94us  10.01ms   67.22%  
    Req/Sec     9.84k     2.23k   25.84k    62.36%  
  1175330 requests in 30.10s, 755.47MB read  
Requests/sec:  39048.31  
Transfer/sec:     25.10MB  
 siddhant  \~  echo epoll\_500  
epoll\_500  
 siddhant  \~  wrk \-t8 \-c500 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  8 threads and 500 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    15.83ms    2.51ms  25.46ms   64.87%  
    Req/Sec     3.92k   673.08     5.64k    74.17%  
  936516 requests in 30.02s, 601.97MB read  
Requests/sec:  31191.37  
Transfer/sec:     20.05MB  
 siddhant  \~  echo epoll\_1000  
epoll\_1000  
 siddhant  \~  wrk \-t8 \-c1000 \-d30s http://localhost:3490/test.html  
Running 30s test @ http://localhost:3490/test.html  
  8 threads and 1000 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    30.02ms    5.45ms  40.53ms   55.02%  
    Req/Sec     4.17k   783.40     6.14k    49.62%  
  995806 requests in 30.03s, 640.08MB read  
Requests/sec:  33155.52  
Transfer/sec:     21.31MB  
 siddhant  \~  

Nginx stats \-   
siddhant  \~  echo nginx\_100  
nginx\_100  
 siddhant  \~  wrk \-t4 \-c100 \-d30s http://localhost:8081/test.html  
Running 30s test @ http://localhost:8081/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency   669.37us  724.81us  14.79ms   87.76%  
    Req/Sec    41.09k    15.51k   91.42k    79.67%  
  4910323 requests in 30.04s, 3.83GB read  
Requests/sec: 163470.07  
Transfer/sec:    130.49MB  
 siddhant  \~  echo nginx\_500  
nginx\_500  
 siddhant  \~  wrk \-t8 \-c500 \-d30s http://localhost:8081/test.html  
Running 30s test @ http://localhost:8081/test.html  
  8 threads and 500 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency     3.03ms    2.13ms  22.86ms   71.35%  
    Req/Sec    20.74k     7.90k   45.59k    76.59%  
  4954843 requests in 30.07s, 3.86GB read  
Requests/sec: 164778.23  
Transfer/sec:    131.53MB  
 siddhant  \~  echo nginx\_1000  
nginx\_1000  
 siddhant  \~  wrk \-t8 \-c1000 \-d30s http://localhost:8081/test.html  
Running 30s test @ http://localhost:8081/test.html  
  8 threads and 1000 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency     6.71ms    3.80ms  52.43ms   71.15%  
    Req/Sec    18.32k     6.17k   41.18k    87.09%  
  4378808 requests in 30.10s, 3.41GB read  
Requests/sec: 145479.65  
Transfer/sec:    116.12MB  
 siddhant  \~  

Fastapi stats \-   
 siddhant  \~  echo fastapi\_100  
fastapi\_100  
 siddhant  \~  wrk \-t4 \-c100 \-d30s http://localhost:8000/test.html  
Running 30s test @ http://localhost:8000/test.html  
  4 threads and 100 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency    42.48ms    1.47ms  58.04ms   78.94%  
    Req/Sec   590.67     26.59   696.00     71.00%  
  70577 requests in 30.01s, 56.16MB read  
Requests/sec:   2351.59  
Transfer/sec:      1.87MB  
 siddhant  \~  echo fastapi\_500  
fastapi\_500  
 siddhant  \~  wrk \-t8 \-c500 \-d30s http://localhost:8000/test.html  
Running 30s test @ http://localhost:8000/test.html  
  8 threads and 500 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency   164.15ms   87.09ms 418.89ms   68.31%  
    Req/Sec   382.03    217.59     1.35k    79.23%  
  91384 requests in 30.08s, 72.74MB read  
Requests/sec:   3037.78  
Transfer/sec:      2.42MB  
 siddhant  \~  echo fastapi\_1000  
fastapi\_1000  
 siddhant  \~  wrk \-t8 \-c1000 \-d30s http://localhost:8000/test.html  
Running 30s test @ http://localhost:8000/test.html  
  8 threads and 1000 connections  
  Thread Stats   Avg      Stdev     Max   \+/- Stdev  
    Latency   315.83ms  109.18ms 634.38ms   63.27%  
    Req/Sec   394.56    186.05     1.17k    75.79%  
  94522 requests in 30.09s, 75.33MB read  
Requests/sec:   3141.77  
Transfer/sec:      2.50MB  
 siddhant  \~  

