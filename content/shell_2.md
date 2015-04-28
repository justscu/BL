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

#### awk
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
```

#### docker
```sh
# 查看所有docker容器
docker ps -a
# 日志
docker logs sds
# 镜像
docker images

# -e 设置环境变量,运行`/bin/bash`程序
# -name 容器的名字
docker run -it -e SERVICE_ID=/root/app_test -e CM=tcp://10.15.144.105:10400 
-e ZK_LIST=10.15.144.105:2181  --name app_test 10.15.108.175:5000/dzhyun/sds:1.0.209 /bin/bash

# -p 端口映射,使用docker运行一个新的zookeeper, `-p 内部端口:外部端口`
docker run --name sds_zookeeper -p 12181:2181 10.15.108.175:5000/library/zookeeper:3.4.6

# 进入到一个正在运行的docker中, 59195为docker中运行的某个进程id
nsenter --target  59195 --mount --uts --ipc --net --pid
```
