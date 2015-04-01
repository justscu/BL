## linux平台操作SQLServer

在Linux平台下操作MS SQLServer，需要安装一些驱动，应用程序通过这些驱动与SQLServer进行交互。

C/golang <---->  unixODBC <----> ODBC driver <----> ODBC database。

[SQL语法教程(w3school)](http://www.w3school.com.cn/sql/index.asp)

[Navicat for SQL Server](http://www.navicat.com.cn/store/navicat-for-sqlserver)，在windows平台下，管理和分析SqlServer。

### 1 ODBC

ODBC(Open Database Connectivity)，开放数据库互连。
**ODBC**是微软公司开放服务结构中有关数据库的一个组成部分，它建立了一组规范，提供了一组对数据库访问的标准API。
这些API利用SQL来完成其大部分任务。ODBC本身也提供了对SQL语言的支持，用户可以直接将SQL语句送给ODBC。

### 2 unixODBC

**unixODBC**是非windows平台下，用来访问ODBC的组件。
下载[unixODBC-2.3.2.tar.gz](http://www.unixodbc.org/)。

```sh
gunzip unixODBC-2.3.2.tar.gz
tar xvf unixODBC-2.3.2.tar
cd unixODBC-2.3.2
./configure
make & sudo make install
```

在/usr/local/etc/下多了2个文件*odbc.ini* & *odbcinst.ini*。

### 3 ODBC driver

**FreeTDS**是unix平台下开源的数据库驱动，可以用来与Microsoft SQL Server & Sybase databases进行交互。
下载[freetds-0.91.tgz](http://www.freetds.org/)。

```sh
tar -zxvf freetds-0.91.tgz
cd freetds-0.91
./configure --with-unixodbc=/usr/local/，假设unixodbc安装在/usr/local目录下，
make & make install
```

在/usr/local/etc目录下，多了*freetds.conf*文件，驱动的位置是/usr/local/lib/libtdsodbc.so。

### 4 修改配置文件

#### 4.1 修改FreeTDS配置

vim /usr/local/etc/freetds.conf，在末尾添加

```sh
[news37014]
        host = xx.xx.xx.xx
        port = 1433
        tds version = 7.0  # if you’re using SQL Server 2000 and above, use version 8.0
		client charset = UTF-8
```

如果你的数据表格中有中文，最好启用client charset = UTF-8，这样可以在客户端显示中文。

测试数据库端口是否可用： **telnet xx.xx.xx.xx 1433**

看看驱动是否正常，测试命令： **tsql -S news37014 -U userName**，见到下面内容，则正常。

```sh
Password:
locale is "en_US.UTF-8"
locale charset is "UTF-8"
using default charset "UTF-8"
1>
```

#### 4.2 修改unixODBC配置

添加驱动的路径。`vim /usr/local/etc/odbcinst.ini`，添加

```sh
[FreeTDS]
	Description=freetds driver
	Driver=/usr/local/lib/libtdsodbc.so
```

添加DSN。`vim /usr/local/etc/odbc.ini`，添加

```sh
[news]
	Driver = FreeTDS
	Description = MSSQL database for my nice app
	Servername = news37014
	Database = dzhdbcenter
```

测试命令：**isql -v news usrname passwd**，会显示下面信息：

```sh
+---------------------------------------+
| |
| |
| sql-statement |
| help [tablename] |
| quit |
| |
+---------------------------------------+
SQL>
```

用**odbcinst -j**命令，可以查看unixODBC的相关信息：

```sh
unixODBC 2.3.2
DRIVERS............: /usr/local/etc/odbcinst.ini
SYSTEM DATA SOURCES: /usr/local/etc/odbc.ini
FILE DATA SOURCES..: /usr/local/etc/ODBCDataSources
USER DATA SOURCES..: /home/ll/.odbc.ini
SQLULEN Size.......: 4
SQLLEN Size........: 4
SQLSETPOSIROW Size.: 2
```

运行程序，若报错，可能是还需要设置环境变量。

报错：**IM002 [unixODBC][Driver Manager]Data source name not found, and no default driver specified**。
解决方法：

```sh
export ODBCINI=/usr/local/etc
export ODBCSYSINI=/usr/local/etc
```

### 5 用golang操作MS SqlServer

#### 5.1 第一种方法，用**go-odbc**库

`git clone https://github.com/weigj/go-odbc.git go-odbc.git`，
测试代码如下：

```go

package main
import (
    "fmt"
    odbc "gw.com.cn/dzhyun/go-odbc.git"
)

func main() {
    fmt.Printf("---\n")
    conn, err := odbc.Connect("DSN=news;UID=usrname;PWD=xxx")
    defer conn.Close()
    if err != nil {
        fmt.Printf("[%s] \n", err)
        return
    }
    stmt, _ := conn.Prepare("select top 10 * from T_table")
    defer stmt.Close()
    stmt.Execute()
    rows, err := stmt.FetchAll()
    if err != nil {
            fmt.Printf("stmt.FetchAll err[%s] \n", err)
    }
    for i, row := range rows {
        fmt.Println(i, row)
    }
}

```

#### 5.2 第二种方法，用**code.google.com/p/odbc**库
`go get code.google.com/p/odbc`，
测试代码如下：

```go

import (
    "fmt"
    "database/sql"
    _ "code.google.com/p/odbc"
    )

func main() {
    params := map[string]string {
        "dsn": "news",
        "uid": "username",
        "pwd": "xxx",
    }

    var c string
    for k, v := range params {
        c += k + "=" + v + ";"
    }

    db, err = sql.Open("odbc", c)
    if err != nil {
        fmt.Printf("sql.Open err[%s] \n", err)
        return
    }

    defer db.Close()

    rows1, err := db.Query("SELECT top 10 act, maxTMSTAMP from T_table")
    if err != nil {
        fmt.Printf("db.Query err[%s] \n", err)
        return
    }

    for rows1.Next() {
        var act string
        var maxTM uint64 = 0
        err := rows1.Scan(&act, &maxTM)
        if err != nil {
            fmt.Printf("rows1.Scan err[%s] \n", err)
            continue
        } else {
            fmt.Printf("act[%c] maxTM[%d] \n", act, maxTM)
        }
    }
}

```

### 6 SQL语句注意事项
- TAMPSTAMP是SqlServer提供的一个计数功能，不能和时间等同；将其转化为bigint的方法：select top 1 convert(bigint, CTAMP) from ... where ...
- select * from t_37 where CTAMP >= convert(timestamp, convert(bigint, 13376385399))，最好先转为bigint再转timestamp，否则会溢出
- 尽量不要在SQL语句中过滤，而是查询出来后，在程序中进行过滤操作。如：select C1, C2, C3 from t37 where C3='a' or C3 = 'b'，不如select C1, C2, C3 from t37的速度快
- 作为查询条件，尽量是主键。非主键作为查询条件，速度会非常慢
