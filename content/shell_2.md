#### cron定时任务

crontab是工具，用来管理定时任务；crond是守护进程，用来执行定时任务。使用`yum install crontabs`命令安装crontab和crond。

```sh
crontab -e  # 修改定时任务
crontab -l  # 显示当前用户有哪些定时任务
crontab -r  # 删除定时任务
service crond start|stop|restart  # 操作crond守护进程
```

修改定时任务`crontab -e`
```sh
# 分 时  日 月 周 task
 40  *  *  *  * /usr/local/bin/runapp.sh >> /dev/null
 45  *  *  *  * ps -aux | grep xxx.app | grep -v "grep" | awk '{print $2}' | xargs kill -9
```

每次编辑完后，cron自动在"/var/spool/cron/"下生成一个与此用户同名的文件，此用户的cron信息都记录在这个文件中，这个文件是不可以直接编辑的，只可以用`crontab -e`来编辑。

每次编辑完后，不需要重启crond守护进程。crond守护进程每分钟都检查"/etc/crontab"、"etc/cron.d/*"、"/var/spool/cron/*"的改变。如果发现了改变，它们就会被载入内存。这样，当某个crontab文件改变后就不必重新启动守护进程了。

注意：在使用crond执行定时任务时，要注意环境变量。脚本执行不起来，有可能是环境变量的原因。
```sh
# 分 时 日 月 周 task
0-59 * * * * date >> a.txt   # 每1分钟执行一次
0-59/3 * * * * date >> a.txt # 每3分钟执行一次
0 0 1,3,5 * *  date >> a.txt #　每月的1,3,5日执行
0-59 * * * *  env >> a.txt 
```
