# Simple Linux Executor Driver


Allow create files with executable context on read

Often you have to use something that relay on local file but you need generate it dynamicaly

This driver act in following way:

echo 'echo time is $(date)' > /dev/executor
cat /dev/executor
sleep 2
cat /dev/executor

See install.sh for details

