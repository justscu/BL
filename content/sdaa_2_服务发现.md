## 系统设计与性能优化之二：服务发现

### 1 服务发现服务的作用
系统中的服务发现能够提供以下一些基本的功能： 
- 能够让后端的服务app接收客户端请求；
- 服务发现能够提供负载均衡，不会使得有些app特别忙，而另一些app特别闲；
- 当后端app不能提供服务时，能够自动将客户端的请求转移到其他app上，这对用户来说是透明的；
- 运维可以很方便的知道后端有哪些服务，以及哪些app可用，哪些app不可用；
- 不存在单点故障；
- 后端app可以卸载，可以直接调试而不用搭建新的调试环境；

[SmartStack](http://nerds.airbnb.com/smartstack-service-discovery-cloud/)是云端的服务注册和服务发现解决方案。包含Nerve和Synapse组件，同时借助于HAProxy，提供负载均衡。整个流程图如下所示： 

![sdaa_2_1](https://github.com/justscu/BL/blob/master/pics/sdaa_2_1.png)

- **Nerve**执行健康检查，若app提供http服务，则向其发送http请求，然后根据反馈的结果判定app状态；若是redis，则向redis写入、读取数据，来判定redis状态。
若app通过健康检查，Nerve就向zk注册临时节点(表明该app可用，以及app的IP:port，服务类型等信息)。若不可用，就删掉该临时节点。
- **Synapse**通过watch临时节点，得到可用的app及其基本信息；并生成新的haproxy.cfg配置文件，重启HAProxy。
- **HAProxy**作为反向代理和负载均衡服务器，接收用户的请求，并将请求发送给较优的app；同时对可用的app作健康检查。

若Nerve或Synapse程序挂掉了，不会有新的app得到注册，但对现有的app不会产生影响。若现有的app挂掉了，HAProxy会通过健康检查得到该app最新状况。 


### 2 服务注册Nerve
Nerve对服务进行健康检查（如发送http包，读写数据库等）。
若服务可用的话，就在zk上注册一个该服务有关的临时节点；若服务变的不可用的话，就将该临时节点删掉。

* （1）下载nerve源码，`git clone https://github.com/airbnb/nerve.git  nerve.git`
* （2）cd nerve.git，将`gem 'nerve'`加到Gemfile的首行
* （3）运行`gem install nerve`，安装nerve
```sh
[xxx102@chef102 nerve]$ sudo gem install nerve
Fetching: zookeeper-1.4.9.gem (100%)
Building native extensions.  This could take a while...
Successfully installed zookeeper-1.4.9
Fetching: little-plugger-1.1.3.gem (100%)
Successfully installed little-plugger-1.1.3
Fetching: logging-1.8.2.gem (100%)Fetching: logging-1.8.2.gem
Successfully installed logging-1.8.2
Fetching: zk-1.9.4.gem (100%)
Successfully installed zk-1.9.4
Fetching: amq-protocol-1.9.2.gem (100%)
Successfully installed amq-protocol-1.9.2
Fetching: bunny-1.1.0.gem (100%)
Successfully installed bunny-1.1.0
Fetching: nerve-0.5.2.gem (100%)
Successfully installed nerve-0.5.2
Parsing documentation for zookeeper-1.4.9
Installing ri documentation for zookeeper-1.4.9
Parsing documentation for little-plugger-1.1.3
Installing ri documentation for little-plugger-1.1.3
Parsing documentation for logging-1.8.2
Installing ri documentation for logging-1.8.2
Parsing documentation for zk-1.9.4
Installing ri documentation for zk-1.9.4
Parsing documentation for amq-protocol-1.9.2
Installing ri documentation for amq-protocol-1.9.2
Parsing documentation for bunny-1.1.0
Installing ri documentation for bunny-1.1.0
Parsing documentation for nerve-0.5.2
Installing ri documentation for nerve-0.5.2
Done installing documentation for zookeeper, little-plugger, logging, zk, amq-protocol, bunny, nerve after 7 seconds
7 gems installed
```
使用`nerve -h`，看看是否安装成功。
*（4）nerver使用json作为配置文件，新建一个test.json文件。
```sh
# 注意：在使用json配置文件的时候，去掉注释
# 确保zk服务器在10.15.107.245:2181上启动；确保nginx在10.15.107.74:19080，10.15.144.71:80上启动
{
    "instance_id": "nginx",
    "service_conf_dir": "",
    "services": {
        "nginx_service1": {
            "host": "10.15.107.74", # nginx服务器的ip
            "port": 19080,          # nginx服务器的port
            "reporter_type": "zookeeper",
            "zk_hosts": ["10.15.107.245:2181"],  # zk服务器
            "zk_path": "/nerve/services/your_http_service/services", # zk上注册的节点
            "check_interval": 2,
            "checks": [ # 检查
             {
                "type": "tcp",
                "uri": "/index.html",
                "timeout": 0.2,
                "rise": 3,
                "fall": 2
             }
            ]
        },
        "nginx_service2": {
            "host": "10.15.144.71",
            "port": 80,
            "reporter_type": "zookeeper",
            "zk_hosts": ["10.15.107.245:2181"],
            "zk_path": "/nerve/services/your_http_service/services",
            "check_interval": 2,
            "checks": [
            {
                "type": "tcp",
                "uri": "/index.html",
                "timeout": 0.2,
                "rise": 3,
                "fall": 2
            }
            ]
        }
    }
}
```
* （5）对配置文件中checks的说明

    * type: (required)作哪种类型的检查。可以在nerve/lib/nerve/service_watcher目录下发现base.rb、http.rb、rabbitmq.rb、tcp.rb文件，支持http/rabbitmq/tcp三种类型。
    * timeout: (optional)检查的最大时间。缺省为100ms
    * rise: (optional) 执行多少次连续成功的检查后，才认为该服务可用。缺省为１
    * fall: (optional) 执行多少次联系失败的检查后，才认为该服务不可用。缺省为１

* （6）运行`nerve -c test.json`，启动nerve程序。若nginx服务正常的话，可以看到zk下建立的临时节点：

![sdaa_2_2](https://github.com/justscu/BL/blob/master/pics/sdaa_2_2.png)

Nerve根据配置文件，对服务进行健康检查。当检查通过时，会创建临时节点（临时节点的值依次增大）。若通不过检查，会删除临时节点。


### 3 服务发现Synapse
Synapse通过在zk上设置watches，来关注zk上特定节点的变化，从而获取服务的状态。
当服务的状态发生变化时（可用/不可用），生成新的Haproxy配置文件，并重新加载HAProxy程序。

* （1）下载源码：`git clone https://github.com/airbnb/synapse.git synapse.git`
* （2）cd synapse.git, 将`gem 'synapse'`加在Gemfile的首行
* （3）安装`gem install synapse`；
```sh
Fetching: excon-0.40.0.gem (100%)
Successfully installed excon-0.40.0
Fetching: docker-api-1.7.6.gem (100%)
Successfully installed docker-api-1.7.6
Fetching: synapse-0.11.1.gem (100%)
Successfully installed synapse-0.11.1
Parsing documentation for excon-0.40.0
Installing ri documentation for excon-0.40.0
Parsing documentation for docker-api-1.7.6
Installing ri documentation for docker-api-1.7.6
Parsing documentation for synapse-0.11.1
Installing ri documentation for synapse-0.11.1
Done installing documentation for excon, docker-api, synapse after 2 seconds
3 gems installed
```
用`synapse -h`来查看是否安装成功。

* （4）Synapse使用json文件作为配置文件，一个典型的配置文件(nerve.cfg.json)如下
```sh
{
    "services": {
        "service_nginx": {
            "discovery": {　 # 发现
                "method": "zookeeper",　 # 方法，有Stub、Zookeeper、Docker、AWS EC2 tags等
                "path": "/nerve/services/your_http_service/services",
                "hosts": [
                    "10.15.107.245:2181"　 # zk配置
                ]
            },
            "haproxy": {   # service_nginx节点下的haproxy配置
                "port": 15021, # 解释　可以将其注释掉，不生成frontend，改为手动生成
                "server_options": "check inter 2s rise 3 fall 2",
                "listen": [
                     "mode http",
                     "option httpchk GET /index.html",
                     "http-check expect status 200"
                ]
            }
        }
    },
    "haproxy": { 　 # haproxy服务器的配置
        "reload_command": "sudo service haproxy reload",  # 当haproxy配置文件发生变化时，reload　Haproxy
        "config_file_path": "/etc/haproxy/haproxy.cfg",
        "socket_file_path": "/var/haproxy/stats.sock",
        "do_writes": true,     # true，重新配置文件
        "do_reloads": true,　  # true，重新加载配置文件
        "do_socket": false,
        "global": [    #  haproxy配置文件中的global项
            "daemon",
            "user haproxy",
            "group haproxy",
            "maxconn 4096",
            "log 127.0.0.1 local0",
            "log 127.0.0.1 local1 notice",
            "stats socket /var/haproxy/stats.sock mode 666 level admin"
        ],
        "defaults": [  # haproxy配置文件中的defaults项
            "log global",
            "option dontlognull",
            "maxconn 2000",
            "retries 3",
            "timeout connect 5s",
            "timeout client 1m",
            "timeout server 1m",
            "option redispatch",
            "balance roundrobin"
        ],
        "extra_sections": { # haproxy额外的配置信息都放在这里
            # haproxy配置文件中的listen项，浏览器通过"xxx:65532/admin?stats"访问haproxy的监控
            "listen admin_status": [
                "bind 0.0.0.0:65532",
                "mode http",
                "log 127.0.0.1 local3 err",
                "stats refresh 5s",             # 5s 刷新一次
                "stats uri /admin?stats",
                "stats realm itnihao\ itnihao", # 监控页面的提示信息
                "stats auth admin:admin"        # 监控页面的用户名和密码
            ]
        }
    }
}
```
* （5）`yum install haproxy`，并使用命令`sudo service haproxy`将HAProxy启动。
* （6）启动`nerve：nerve -c nerve.cfg.json`
* （7）当服务的状态发生变化时，Nerve会改变zk上相应节点的状态。Synapse会感知到，并生成新的HAProxy的配置文件。并重新加载HAProxy程序。一个生成的配置文件如下
```sh
# auto-generated by synapse at 2014-11-05 14:54:19 +0800

global
        daemon
        user haproxy
        group haproxy
        maxconn 4096
        log 127.0.0.1 local0
        log 127.0.0.1 local1 notice
        stats socket /var/haproxy/stats.sock mode 666 level admin
defaults
        log global
        option dontlognull
        maxconn 2000
        retries 3
        timeout connect 5s
        timeout client 1m
        timeout server 1m
        option redispatch
        balance roundrobin

listen admin_status
        bind 0.0.0.0:65532
        mode http
        log 127.0.0.1 local3 err
        stats refresh 5s
        stats uri /admin?stats
        stats realm itnihao itnihao
        stats auth admin:admin

frontend service_nginx
        mode http
        bind localhost:15021
        default_backend service_nginx

backend service_nginx
        mode http
        option httpchk GET /index.html
        http-check expect status 200
        server 10.15.107.74:19080_nginx 10.15.107.74:19080 cookie 10.15.107.74:19080_nginx check inter 2s rise 3 fall 2
        server 10.15.144.71:80_nginx 10.15.144.71:80 cookie 10.15.144.71:80_nginx check inter 2s rise 3 fall 2
```
* （８）对nerve.cfg.json配置文件中port字段的解释。
在`https://github.com/airbnb/synapse`中有句这样的话：
*port*: the port (on localhost) where HAProxy will listen for connections to the service. If this is omitted, only a backend stanza (and no frontend stanza) will be generated for this service; you'll need to get traffic to your service yourself via the *shared_frontend* or manual frontends in *extra_sections*。

其表达的含义为：若使用port字段("port": 15021)，最终在HAProxy的配置文件中，会生成bind localhost:15021，这样只能够在本机进行访问该端口，在非本机访问会返回connection refused。若非本机要想访问该端口，需要在生成配置文件时，使用shared_frontend或extra_sections字段。
```sh
# 使用"port":15021, 生成的HAProxy配置文件片段
frontend service_nginx
　　　　mode http
　　　　bind localhost:15021　# localhost:15021只有本机可以访问，其他机器访问，会返回connect refused
　　　　default_backend service_nginx
```
若省略port字段，就不会生成frontend的相关配置，但你可以在extra_sections字段中手动添加
```sh
# synapse.cfg.json配置文件片段，不使用"port":15021，而是在"extra_sections"中额外手动添加"frontend service_nginx"配置
"extra_sections": {
  "listen admin_status": [
      "bind 0.0.0.0:65532",
      "mode http",
      "log 127.0.0.1 local3 err",
      "stats refresh 5s",
      "stats uri /admin?stats",
      "stats realm itnihao\ itnihao",
      "stats auth admin:admin"
  ],
  "frontend service_nginx": [
      "mode http",
      "bind 0.0.0.0:15021",  # 任何ip都可以访问
      "default_backend service_nginx"
  ]
}
```
最终生成的HAProxy配置文件片段
```sh
frontend service_nginx
    mode http
    bind 0.0.0.0:15021
    default_backend service_nginx
```
