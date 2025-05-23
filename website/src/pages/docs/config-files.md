---
title: "Config Files"
---

## Config Files

### scope.yml Config File

Below are the default contents of the `scope.yml` library configuration file:

```
---
metric:
  enable: true                      # true, false
  format:
    type : statsd                   # statsd, ndjson
    #statsdprefix : 'cribl.scope'    # prepends each statsd metric
    statsdmaxlen : 512              # max size of a formatted statsd string
    verbosity : 4                   # 0-9 (0 is least verbose, 9 is most)
          # 0-9 controls which expanded tags are output
          #      1 "data"
          #      1 "unit"
          #      2 "class"
          #      2 "proto"
          #      3 "op"
          #      4 "host"
          #      4 "proc"
          #      5 "domain"
          #      5 "file"
          #      6 "localip"
          #      6 "remoteip"
          #      6 "localp"
          #      6 "port"
          #      6 "remotep"
          #      7 "fd"
          #      7 "pid"
          #      8 "duration"
          #      9 "numops"
          #
          # 5-9 disables event summarization of different kinds
          #      5  turns off error event summarization
          #      6  turns off filesystem open/close event summarization
          #      6  turns off dns event summarization
          #      7  turns off filesystem stat event summarization
          #      7  turns off network connect event summarization
          #      8  turns off filesystem seek event summarization
          #      9  turns off filesystem read/write summarization
          #      9  turns off network send/receive summarization
    tags:
      #user: $USER
      #feeling: elation
  transport:                        # defines how scope output is sent
    type: udp                       # udp, tcp, unix, file, syslog
    host: 127.0.0.1
    port: 8125

event:
  enable: true                      # true, false
  transport:
    type: tcp                       # udp, tcp, unix, file, syslog
    host: 127.0.0.1
    port: 9109
  format:
    type : ndjson                   # ndjson
    maxeventpersec: 10000           # max events per second.  zero is "no limit"
    enhancefs: true                 # true, false
  watch:
    # Creates events from data written to files.
    # Designed for monitoring log files, but capable of capturing
    # any file writes.
    - type: file
      name: .*log.*                 # allowlist ex regex describing log file names
      value: .*                     # allowlist ex regex describing field values

    # Creates events from data written to stdout, stderr, or both.
    # May be most useful for capturing debug output from processes
    # running in containerized environments.
    - type: console
      name: stdout                  # (stdout|stderr)
      value: .*

    # Creates events from libscope's metric (statsd) data.  May be
    # most useful for data which could overwhelm metric aggregation
    # servers, either due to data volumes or dimensions (tags) with
    # high cardinality.
#    - type: metric
#      name: .*                      # (net.*)|(.*err.*)
#      field: .*                     # allowlist regex describing field names
#      value: .*

    # Enable extraction of HTTP headers
    - type: http
      name: .*                      # (http-resp)|(http-metrics)
      field: .*                     # allowlist regex describing field names
      value: .*
      headers:                      # (?i)X-myHeader.*

    # Creates events describing network connectivity
    - type: net
      name: .*                      #
      field: .*                     # allowlist regex describing field names
      value: .*

    # Creates events describing file connectivity
    - type: fs
      name: .*                      #
      field: .*                     # allowlist regex describing field names
      value: .*

    # Creates events describing dns activity
    - type: dns
      name: .*                      #
      field: .*                     # allowlist regex describing field names
      value: .*

payload:
  enable: false                     # true, false
  dir: '/tmp'

libscope:
  configevent: true                 # true, false
  summaryperiod : 10                # in seconds
  commanddir : '/tmp'
  #  commanddir supports changes to configuration settings of running
  #  processes.  At every summary period the library looks in commanddir
  #  to see if a file named scope.<pid> exists.  (Where pid is the process ID
  #  of the process running with scope.)  If it exists, it processes every
  #  line for environment variable-style commands:
  #      SCOPE_METRIC_VERBOSITY=9
  #  It changes the configuration to match the new settings, and deletes the
  #  scope.<pid> file when it's complete.

  log:
    level: warning                    # debug, info, warning, error, none
    transport:
      type: file
      path: '/tmp/scope.log'
      buffering: line               # line, full
...
```

#### Using TLS

To use TLS for secure communication, edit the `transport` element such that `type` is set to `tcp`, and `tls : enable` is set to `true`, as shown in the example below. The remaining elements in the example can be configured to suit your environment. See [TLS](/docs/tls).

```
  transport:
      type: tcp
      host: 127.0.0.1
      port: 10090
      tls:
        enable: true
        validateserver: false         # false is like curl's -k option
        cacertpath: ''
            # Use this field if you want to specify a private CA.
            # Leave it empty if you want AppScope to use public root certs
            # provided by the Linux distro.

```

### scope_protocol.yml Config File

Below are the default contents of the `scope_protocol.yml` library configuration file: 

```
---
protocol:
  - name: Redis
    binary: false
    regex: "^[*]\\d+|^[+]\\w+|^[$]\\d+"

  - name: Mongo
    binary: true
    regex: "^240100000000000000000000d407"
    len: 32
---
```
