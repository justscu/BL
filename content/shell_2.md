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


#### 2. awk
```sh
#过滤含有Reponse info的行, 正则表达式将由 '/ /' 包裹
awk '/Reponse info/' proxy.06-20.log;      
#过滤不含有Reponse info的行
awk '!/Reponse info/' proxy.06-20.log;
#两个条件进行过滤(与)
awk '/Request/;/Reponse/'  proxy.06-20.log
#使用多个条件进行过滤
awk '/1164691776/&&(/Request/ || /Reponse/)' proxy.06-20.log
#使用正则表达式作为过滤条件
awk '/1164691776/&&(/Request/||/Reponse/)　{if($2 ~ /^11/) print $0}'
awk '($2 ~ /^02:02/) {print $0}'   proxy.06-20.log
#统计多少行
awk '($2 ~ /^02:02/)  {count++} END{print count}'  proxy.06-20.log
#使用长度作为条件
awk '/Reponse info/ {if (length($7) > 17) print $2, $7}' proxy.06-20.log
#使用[作为分隔符， 默认用空格作为分隔符
awk -F '['   '/socket close/ {print $4}' proxy.06-25.log | sort | uniq
#统计各状态的个数
netstat -ano | awk '/^tcp/ {t[$6]++} END{for(state in t) {print state, t[state]} }'
#有多行这种数据，2015-09-10 10:20:28 [info] synco: brand(1000)，求()中数字之和
cat system.log | awk '/synco/ {print $5}' | awk -F '(' '{print $2}' | awk -F ')' '{print $1}' | awk '{a+=$0} END{print a}'
#改变某列的值(把第二列设置为空)
awk '{print $2=null,$0}' system.log

```


#### 3. vim查找与替换
1.查找单词<br/> 
把光标移动到单词上: (1)只查单词，按下`*`；(2)模糊匹配，按下`g*`

2.查找<br/>
在normal模式下，可以使用正则表达式进行查找。
```
`/abc`,不分大小写，匹配所有
`/^abc`，abc开头
`/abc$`，abc结尾
`/abc/c`,大小写不敏感
`/abc/C`，大小写敏感
大小写敏感，也可以设置`set smartcase`; 大小写不敏感，也可以设置`set ignorecase`
```

3.模式匹配<br/>
```
`/[vim]`，匹配v,i,m中之一
`/[a-zA-z]`，匹配任意字母
`/[1-5]`，匹配1-5间的数字
`/.`，匹配任意字符
`/vi*m`，`*`前字符出现大于等于0次。如vm, vim, viim, viiim
`/vi\+m`，`+`前字符出现大于等于1次。如vim, viim, viiim
`/vi\?m`, `?`前字符出现0次或一次。如vm, vim
`/^vim`, 匹配开头
`/vim$`，匹配结尾
`/vim\|kv`, 匹配vim或kv
```

4.替换<br/>
`:{作用范围}s/{目标}/{替换}/{替换标识}`
 
当前行: `:s/foo/bar/g` <br/>
全文: `:%s/foo/bar/g` <br/>
第4-10行: `:4,10s/foo/bar/g` <br/>
当前行和接下来2行: `:.,+2s/foo/bar/g` <br/>

`g`，global,全局替换；`i`，大小写不敏感； `I`，大小写敏感； `c`, 需要确认

`:s/foo/bar/gI`等价与`:s/foo\C/bar/g`

去掉windows格式末尾的\r\n, `:%s/^M//g`, 在命令中，^M的输入方式为: Ctrl+v, Ctrl+m,是一个字符，不是两个.


#### 4 守护脚本
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



#### 5. docker
docker中有**镜像(image)/容器(container)/仓库(hub)**的概念。

```sh
# 镜像
# (1)显示本地镜像
docker images
# (2)删除镜像
docker rmi imagename

# 容器
# (1)启动
docker run -t -i imagename /bin/bash
docker start/stop imagename
# (2)查看所有docker容器
docker ps -a
# (3)日志
docker logs sds

# -e 设置环境变量,运行`/bin/bash`程序
# -name 容器的名字
docker run -it -e SERVICE_ID=/root/app_test -e CM=tcp://10.15.144.105:10400 
-e ZK_LIST=10.15.144.105:2181  --name app_test 10.15.108.175:5000/dzhyun/sds:1.0.209 /bin/bash

# -p 端口映射,使用docker运行一个新的zookeeper, `-p 内部端口:外部端口`
docker run --name sds_zookeeper -p 12181:2181 10.15.108.175:5000/library/zookeeper:3.4.6

# 进入到一个正在运行的docker中, 59195为docker中运行的某个进程id
nsenter --target  59195 --mount --uts --ipc --net --pid
```



#### 6. 用本地iso文件作为yum源
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



#### 7. 批量删除redis中特定的key
`eval "redis.call('del', unpack(redis.call('keys','XinWenXinXi3*')))" 0`

`./redis-cli keys "XinWenXinXi*" | xargs ./redis-cli del`



#### 8. 查看二进制文件
`vim -b xxx.txt`

`xxd xxx.txt`，可以看到16进制和文本文件的数据

`hexdump xxx.txt`


#### 9. osx上terminal合适的字体
`描述文件->Pro；文本->字体(Monaco 14磅)`


