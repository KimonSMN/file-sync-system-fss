
# File Sync System (FSS)

A UNIX-based distributed file synchronization system implemented in C for the Spring 2025 System Programming course (K24). 
It supports parallel file syncing using worker processes, directory monitoring via `inotify`, and interprocess communication through named pipes.

## 🛠️ Compilation

To build the project, simply run:

```bash
make
```

## 🚀 Execution

To run the FSS manager:

```bash
./build/fss_manager -l <path_to_manager_log> -c <path_to_config_file> -n <number_of_workers>
```

Example:

```bash
./build/fss_manager -l logs/manager.log -c config.txt -n 5
```

## 🧠 Implementation Notes

- **sync_info_mem_store**: Implemented as a global hash table to allow efficient and concurrent access by multiple parts of the system.
- **Task Queue**: A global queue stores synchronization tasks when the system is at its worker limit, ensuring no tasks are lost.
- **Signal Safety**: `SIGCHLD` is blocked temporarily during parsing of the config file to avoid race conditions with prematurely spawned workers.
- **Worker Limiting**: The manager enforces a limit on concurrent worker processes and queues excess sync jobs.
- **Named Pipes**: Used for bidirectional communication between `fss_console` and `fss_manager`.

## ⚠️ Limitations

- ❌ `exec_report` was not fully implemented. Workers do not send back detailed sync reports (e.g., `[TIMESTAMP] [SOURCE_DIR] [TARGET_DIR] [WORKER_PID] [OPERATION] [RESULT] [DETAILS]`).
- ❌ `fss_script.sh` is partially implemented — commands like `listAll` and `listMonitored` are **not yet functional**.
- ❌ Parallel sync operations break if workers block on `read()` — workaround involved commenting out some `spawn_worker()` logic.

## 📁 Config File Format

Each line must contain a pair of paths: `<source_dir> <target_dir>`, e.g.:

```
/home/user/docs /backup/docs
/home/user/photos /backup/photos
```