# Simple Linux Executor Driver


Allow to create files with executable context on read event

Often you have to use something that rely on local file, but you need generate it dynamicaly

This driver act in following way:

```
echo 'echo time is $(date)' > /dev/executor
cat /dev/executor
sleep 2
cat /dev/executor
```

See install.sh for details
