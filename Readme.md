# Simple Linux Executor Driver


Allow to create files with executable context on read event

Often you have to use something that rely on local file, but you need generate it dynamicaly

This driver act in following way:

```bash
echo 'echo time is $(date)' > /dev/executor
cat /dev/executor #Fri May  4 21:21:04 EEST 2018
sleep 2
cat /dev/executor #Fri May  4 21:21:06 EEST 2018
```

See install.sh for details
