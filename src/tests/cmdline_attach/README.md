# cmdline_attach

Regression test for command-line attach startup handling.

The Python driver starts `cmdline_attach.exe`, waits for its ready line, then launches:

```text
headless.exe -testing -p <pid> -cf test.txt
```

The script asserts that the attach command completed and set `$pid`/`$hp`, then detaches. A startup attach freeze is reported as a headless timeout.
