#### 1. cron定时任务

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

```sh
# 分 时 日 月 周 task
0-59 * * * * date >> a.txt   # 每1分钟执行一次
0-59/3 * * * * date >> a.txt # 每3分钟执行一次
0 0 1,3,5 * *  date >> a.txt # 每月的1,3,5日执行
0-59 * * * *  env >> a.txt 
```

注意：在使用crond执行定时任务时，要注意环境变量。脚本执行不起来，有可能是环境变量的原因。
想看脚本执行的效果，可以将结果定向输出到文件。如
```sh
* * * * * /tmp/xxx.sh >>/tmp/test.log 2>&1
```



#### 2. 守护脚本
```sh
#!/bin/bash

while true
do
    sleep 2
    line=`ps -aux | grep svc.app | grep -v "grep" | wc -l`
    if [ ${line} = 0 ]; then
        date >> re.log
        echo restart again >> re.log
        ./b.sh # shell need to run
    else
        echo "on running"
    fi
done
```



#### 3. 用本地iso文件作为yum源
创建本地yum源，`mkdir /mnt/iso`, `mkdir /mnt/cdrom`；

将ios文件做个软链接到`/mnt/iso`，`ln -s /home/ll/software/iso/rhel-server-7.2-x86_64-dvd.iso /mnt/iso`；

将`/mnt/ios`下的iso文件，挂载到`/mnt/cdrom`, `mount -o loop -t iso9660 /mnt/iso/rhel-server-7.2-x86_64-dvd.iso /mnt/cdrom`, 使用`df -h`命令，查看是否挂载成功；

修改`/etc/yum.repos.d/`下的`*.repo`文件, 可以先将原来`/etc/yum.repos.d`目录进行备份，然后新建文件，`touch my.repo, vim my.repo`，添加内容：

```
[base]
name=RedHat7.2
baseurl=file:///mnt/cdrom
enable=1
gpgcheck=0
gpgkey=file:///mnt/cdrom/RPM-GPG-KEY-redhat-release
```
	
注意：这个地方一定不要用redhat.repo，该文件会被覆盖

用`yum repolist all`命令看看yum源是否可用



#### 4. 查看二进制文件
`vim -b xxx.txt`

`xxd xxx.txt`，可以看到16进制和文本文件的数据

`hexdump xxx.txt`



#### 5. osx上terminal合适的字体
`描述文件->Pro；文本->字体(Monaco 14磅)`


