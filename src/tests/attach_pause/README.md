# attach_pause

Regression test for pausing an attached debuggee.

The Python driver starts `attach_pause.exe` (a message pump main thread plus
parked worker threads), then sends `attach .<pid>` and `pause` through the
headless interactive command handler. This uses `DbgCmdExec` like the GUI, so
the attach does not pause the debuggee.

After the attach settles, the driver makes a worker thread emit
`OutputDebugString` and block forever, which turns that worker into the
debugger's active thread. The `pause` command must interrupt the debuggee
anyway by targeting the main thread instead of the (parked) active thread. The
driver asserts that the debugger reaches the paused state within a timeout.
