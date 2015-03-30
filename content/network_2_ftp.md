## 文件传输协议(ftp)

### 1 什么是ftp 
ftp(File Transfer Protocol)属于TCP/IP族的应用层协议，使用TCP进行传输，用于网络上的两台主机之间传输文件。
ftp分为客户端和服务器两个软件。
```sh
# 服务器的安装
sudo apt-get install vsftpd
#启动
service vsftpd start|restart|stop
# 修改服务器的配置
vim /etc/vsftpd.conf。在配置文件中，有对用户登录的限制、也有对用户权限的限制。
# 客户端的安装
sudo apt-get install ftp
```
[权限配置参考](http://blog.chinaunix.net/uid-24625974-id-2845256.html)

 ![image](https://github.com/justscu/BL/blob/master/pics/network_2_1.png)
 
服务器运行于20和21两个端口（以主动模式为例）。20端口用于c/s间传输数据流（如上传/下载文件等）；21端口用于c/s间传输控制流（如ftp命令LIST/PWD等）。

当数据通过数据流进行传输时，控制流处于空闲状态。当控制流空闲很长时间时，防火墙会将控制流的会话设置为超时。

当大量数据进行传输时，需要占用大量的时间，控制流处于空闲状态。虽然文件可以传输成功，但控制流会被防火墙关闭，导致产生一些错误。


### 2 ftp的两种工作模式 
ftp分为主动模式和被动模式。

**主动模式**要求客户端和服务器都各打开一个监听端口以建立连接。其步骤为：
- （1）服务器打开监听端口21。客户端打开一个随机端口X，并向服务器端口21发送连接请求，并建立连接（建立控制流）。
- （2）客户端打开监听端口X+1，并通过控制流向服务器发送一个端口命令(PORT)。告诉服务器端口X+1准备好了，可以建立连接。
- （3）服务器打开20端口，并通过20端口连接客户端的X+1端口（建立数据流）。

**被动模式**要求服务器端打开一个监听端口即可。控制流和数据流的建立，都由客户端发起。其步骤为：

 ![image](https://github.com/justscu/BL/blob/master/pics/network_2_2.png)
 
- （1）客户端打开两个本地端口N和N+1。
- （2）客户端用N端口去连接服务器的21端口（建立控制流）。
- （3）客户端通过控制流向服务器发送一个PASV命令（不再是PORT命令）。服务器生成一个随机端口K，并向客户端发送一个PORT命令，告之客户端其生成了一个随机端口K。
- （4）客户端通过N+1端口，向服务器的K端口发起连接请求并建立连接（建立数据流）。

注意：
- （1）PORT命令的格式为：“PORT 10,15,107,74,14,178”，中间用逗号隔开，10.15.107.74是IP地址，端口号=14*256+178。
- （2）所有的命令，通过控制流发送；LIST命令返回的数据，通过数据流发送。在发送完数据后，数据流通道是要断开的。下次再发送数据时，会重新建立新的数据流通道。


### 3 使用ftp带来的问题 
- （1）用户名、密码、文件等都使用明文传输
- （2）主动模式对ftp服务器有利，主动模式下，服务器开启20和21两个端口。但对ftp客户端不利，因为ftp服务器会去连客户端的随机端口，很可能会被客户端的防火墙阻塞掉。
- （3）被动模式对ftp客户端有利。但对ftp服务器不利，因为服务器开启的随机端口等待客户端连接，可能被服务器端的防火墙阻塞掉。通常可以限制服务器随机端口的范围来减少这种问题。

### 4 基本命令 
常用的基本命令有：help, open, passive, binary, ls, put, get等。

 ![image](https://github.com/justscu/BL/blob/master/pics/network_2_3.png)


### 5 用户权限 
#### 5.1 添加新用户
```sh
// 添加新用户，假设用户名为yunftp
useradd yunftp -s /sbin/nologin -d /home/yunftp #不能够立即使用
useradd yunftp -s /bin/bash -d /home/yunftp #能够立即使用 
passwd yunftp #设置密码
 # 遇到“This account is currently not available”问题，vim /etc/passwd，将/sbin/nologin改为/bin/bash。
 # yunftp:x:513:513::/home/yunftp:/sbin/nologin 修改为 yunftp:x:513:513::/home/yunftp:/bin/bash
```

#### 5.2 ftp服务器权限配置
在`/etc/vsftpd/`目录下，有三个文件（ftpusers，user_list，vsftpd.conf），决定了用户的权限。

ftpusers：静止使用ftp服务器的用户列表。里面为禁止用户的用户名。

user_list：里面放置了一些用户名。当`userlist_deny=NO`（vsftpd.conf中修改）时，仅允许本列表中的用户访问ftp服务器。当`userlist_deny=YES`（默认）时，不允许本列表中的用户访问ftp服务器。

vsftpd.conf：是否允许用户登陆及用户权限的配置。
```sh
anonymous_enable=YES，允许匿名用户登陆
local_enable=YES，允许非匿名用户登陆
write_enable=YES，允许非匿名用户写
local_umask=022，非匿名用户写文件的属性 
```
```sh
# permission denied的例子
ftp> open 10.15.107.74 21
Connected to 10.15.107.74.
220 (vsFTPd 2.0.5)
Name (10.15.107.74:ll): yunftp
530 Permission denied.
Login failed. 
```
修改完配置后，重启ftp服务器：`service vsftpd restart`。

让ftp服务在每次开机后自动启动：`sudo vim /etc/rc.d/rc.local`，在最后加上"./usr/sbin/vsftpd"。


### 6 示例代码 
示例代码，是用golang开发的一个ftp客户端，能够完成`LIST/PUT/GET`等基本命令。

[代码地址](https://github.com/justscu/goftp)

