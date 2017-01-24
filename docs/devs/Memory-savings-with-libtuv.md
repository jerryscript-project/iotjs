Memory usage with libtuv on iotjs is described here and compared to libuv.

* Compared with release version in i686.
* iotjs version: hash f8e8391d8c30a76c2f82644e454056c11a2bad1a

#### runtime memory usage compare with libuv running iotjs

1) how to build

1-1) with libuv
```
./tools/build.py --buildtype=release --nochecktest 
```
1-2) with libtuv
```
./tools/build.py --buildtype=release --nochecktest --tuv
```
2) memory usage measurement with valgrind running `test httpserver`
```
valgrind ./build/i686-linux/release/iotjs/iotjs ./test/run_pass/test_httpserver.js
```

2-1) with libuv
```
==5740== Memcheck, a memory error detector
==5740== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
==5740== Using Valgrind-3.10.0.SVN and LibVEX; rerun with -h for copyright info
==5740== Command: ./build/i686-linux/release/iotjs/iotjs ./test/run_pass/test_httpserver.js
==5740== 
==5740== 
==5740== HEAP SUMMARY:
==5740==     in use at exit: 0 bytes in 0 blocks
==5740==   total heap usage: 959 allocs, 959 frees, 482,669 bytes allocated
==5740== 
==5740== All heap blocks were freed -- no leaks are possible
==5740== 
==5740== For counts of detected and suppressed errors, rerun with: -v
==5740== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

2-2) with libtuv
```
==7584== Memcheck, a memory error detector
==7584== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
==7584== Using Valgrind-3.10.0.SVN and LibVEX; rerun with -h for copyright info
==7584== Command: ./build/i686-linux/release/iotjs/iotjs ./test/run_pass/test_httpserver.js
==7584== 
==7584== 
==7584== HEAP SUMMARY:
==7584==     in use at exit: 0 bytes in 0 blocks
==7584==   total heap usage: 955 allocs, 955 frees, 481,645 bytes allocated
==7584== 
==7584== All heap blocks were freed -- no leaks are possible
==7584== 
==7584== For counts of detected and suppressed errors, rerun with: -v
==7584== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

482,669 vs 481,645 = 1,024 bytes saved

#### binary size

1) build
```
./tools/build.py --buildtype=release --nochecktest --target-arch=arm --target-os=linux
./tools/build.py --buildtype=release --nochecktest --target-arch=arm --target-os=linux --tuv
```

2) binary size in i686-linux

* libuv : 213,130 / iotjs: 2,512,292, stripped: 782,152
* libtuv: 103,158 / iotjs: 2,460,357, stripped: 732,776
* itself: 109,972 smaller, iotjs: 51,935(striped 49,376) saved

3) binary size in arm-linux

* libuv : 176,614 / iotjs: 2,543,525, stripped: 536,460
* libtuv:  83,458 / iotjs: 2,506,455, stripped: 507,548
* itself:  93,156 smaller, iotjs:    37,070(stripped: 28,912) saved

#### libuv vs libtuv itself

1) use `tuvtester` as an application to compare.

2) codes changes to make both libuv.a and libtuv.a interchangeable. 

2-1) as libtuv uses c++ compiler, wrap all codes with
```
extern "C" {
...
}
```
2-2) for libuv, give `libuv.a` by changing `tuvtest.cmake` file. use file from iotjs build.
```
target_link_libraries(${TUVTESTNAME} LINK_PUBLIC
                      #${TARGETLIBNAME}
                      "/(absolute path to libuv)/libuv.a"
                      ${TUV_LINK_LIBS})
```

2-3) some functions that does not exist in libuv. add this bottom of in runner_main.cpp
```
#if 1

#define uv__handle_deinit(h)                                                  \
  do {                                                                        \
    QUEUE_REMOVE(&(h)->handle_queue);                                         \
    QUEUE_INIT(&(h)->handle_queue);                                           \
  }                                                                           \
  while (0)

void uv_deinit(uv_loop_t* loop, uv_handle_t* handle) {
  QUEUE* q;
  uv_handle_t* h;

  QUEUE_FOREACH(q, &loop->handles_queue) {
    h = QUEUE_DATA(q, uv_handle_t, handle_queue);
    if (h == handle) {
      uv__handle_deinit(handle);
      break;
    }
  }
}

#endif
```

2-4) remove test codes that does not run with libuv, tested codes are like this in runner_list.h
```
#define TEST_LIST_ALL(TE)                                                     \
  TE(idle_basic, 5000)                                                        \
  TE(timer_init, 5000)                                                        \
  \
  TE(condvar_2, 5000)                                                         \
  TE(condvar_3, 5000)                                                         \
  TE(cwd, 5000)                                                               \
  \
  TE(fs_file_noent, 5000)                                                     \
  TE(fs_file_sync, 5000)                                                      \
  TE(fs_file_async, 5000)                                                     \
  TE(fs_file_write_null_buffer, 5000)                                         \
  TE(fs_stat_missing_path, 5000)                                              \
  TE(fs_open_dir, 5000)                                                       \
  TE(fs_file_open_append, 5000)                                               \
  TE(fs_read_file_eof, 5000)                                                  \
  \
  TE(threadpool_queue_work_simple, 5000)                                      \


// shutdown_eof should be last of tcp test, it'll stop "echo_sevrer"

#if defined(__linux__) 
#define TEST_LIST_EXT(TE)                                                     \

#else
#define TEST_LIST_EXT(TE)                                                     \

#endif

#define HELPER_LIST_ALL(TE)                                                   \

```

3) measure

run with valgrind
```
valgrind ./build/i686-linux/release/bin/tuvtester
```

3-1) with libuv
```
==24952== Memcheck, a memory error detector
==24952== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
==24952== Using Valgrind-3.10.0.SVN and LibVEX; rerun with -h for copyright info
==24952== Command: ./build/i686-linux/release/bin/tuvtester
==24952== 
Run Helpers...
[idle_basic                    ]...OK
[timer_init                    ]...OK
[condvar_2                     ]...OK
[condvar_3                     ]...OK
[cwd                           ]...OK
[fs_file_noent                 ]...OK
[fs_file_sync                  ]...OK
[fs_file_async                 ]...OK
[fs_file_write_null_buffer     ]...OK
[fs_stat_missing_path          ]...OK
[fs_open_dir                   ]...OK
[fs_file_open_append           ]...OK
[fs_read_file_eof              ]...OK
[threadpool_queue_work_simple  ]...OK
Waiting Helpers to end...
==24952== 
==24952== HEAP SUMMARY:
==24952==     in use at exit: 0 bytes in 0 blocks
==24952==   total heap usage: 44 allocs, 44 frees, 1,727 bytes allocated
==24952== 
==24952== All heap blocks were freed -- no leaks are possible
==24952== 
==24952== For counts of detected and suppressed errors, rerun with: -v
==24952== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

3-2) with libtuv
```
==26621== Memcheck, a memory error detector
==26621== Copyright (C) 2002-2013, and GNU GPL'd, by Julian Seward et al.
==26621== Using Valgrind-3.10.0.SVN and LibVEX; rerun with -h for copyright info
==26621== Command: ./build/i686-linux/release/bin/tuvtester
==26621== 
Run Helpers...
[idle_basic                    ]...OK
[timer_init                    ]...OK
[condvar_2                     ]...OK
[condvar_3                     ]...OK
[cwd                           ]...OK
[fs_file_noent                 ]...OK
[fs_file_sync                  ]...OK
[fs_file_async                 ]...OK
[fs_file_write_null_buffer     ]...OK
[fs_stat_missing_path          ]...OK
[fs_open_dir                   ]...OK
[fs_file_open_append           ]...OK
[fs_read_file_eof              ]...OK
[threadpool_queue_work_simple  ]...OK
Waiting Helpers to end...
==26621== 
==26621== HEAP SUMMARY:
==26621==     in use at exit: 0 bytes in 0 blocks
==26621==   total heap usage: 40 allocs, 40 frees, 991 bytes allocated
==26621== 
==26621== All heap blocks were freed -- no leaks are possible
==26621== 
==26621== For counts of detected and suppressed errors, rerun with: -v
```

3-3) result

* libuv: 1,727 bytes
* libtuv:  991 bytes