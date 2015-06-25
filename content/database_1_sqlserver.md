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
- `TAMPSTAMP`是SqlServer提供的一个计数功能，不能和时间等同；将其转化为`bigint`的方法：select top 1 convert(bigint, CTAMP) from ... where ...
- select * from t_37 where CTAMP >= convert(timestamp, convert(bigint, 13376385399))，最好先转为`bigint`再转`timestamp`，否则会溢出
- 尽量不要在SQL语句中过滤，而是查询出来后，在程序中进行过滤操作。如：`select C1, C2, C3 from t37 where C3='a' or C3 = 'b'`，不如`select C1, C2, C3 from t37`的速度快
- 作为查询条件，尽量是主键。非主键作为查询条件，速度会非常慢
- 一个并表查询的例子：`select A.C1, A.C2, A.C3, A.C4, A.C5, A.C6, A.C7, B.C1, B.C2, B.C3, B.C4 from (select C1, C2, C3, C4, C5, C6, C7 from TQ_SK_1 where TQ_SK_1.C1 = 1705408) as A inner join TQ_SK_2 as B on A.C2 = B.C2`


### 7 修正代码
`code.google.com/p/odbc/column.go`的Value()的`c.BaseColumn.Value(c.Buffer[:c.Len])`，经常会panic，修正代码为
```go
func (c *BindableColumn) Value(h api.SQLHSTMT, idx int) (driver.Value, error) {
    if !c.IsBound {
        ret := c.Len.GetData(h, idx, c.CType, c.Buffer)
        if IsError(ret) {
            return nil, NewError("SQLGetData", h)
        }
    }
    if c.Len.IsNull() {
        // is NULL
        return nil, nil
    }

    if c.Len < 0 {
        fmt.Printf("code.google.com/p/odbc/column.go::value() error c.Len[%d]\n", c.Len)
        return nil, nil
    }

    if !c.IsVariableWidth && int(c.Len) != c.Size {
        panic(fmt.Errorf("wrong column #%d length %d returned, %d expected", idx, c.Len, c.Size))
    }
    return c.BaseColumn.Value(c.Buffer[:c.Len])
}
```

### 8 字符集不同导致客户端进入死循环

数据源是MS SqlServer，客户端使用`code.google.com/p/odbc` + `unixODBC-2.3.2` + `freetds-0.91`。
发现在某些测试环境下，内存会快速上涨(200M/s)，直到把内存撑爆。

使用go的`pprof`工具，发现有个函数消耗的内存大于5GB，
```sh
code.google.com/p/odbc.(*NonBindableColumn).Value　5171.3 (99.9%)
```
基本上可以定位该函数中出现了问题，吃了大量内存。代码(code.google.com/p/odbc/column.go:259行)如下：
```go
func (c *NonBindableColumn) Value(h api.SQLHSTMT, idx int) (driver.Value, error) {
	var l BufferLen
	var total []byte
	b := make([]byte, 1024)
loop:
	for {
		ret := l.GetData(h, idx, c.CType, b)
		switch ret {
		case api.SQL_SUCCESS:
			if l.IsNull() {
				// is NULL
				return nil, nil
			}
			if int(l) > len(b) {
				return nil, fmt.Errorf("too much data returned: %d bytes returned, but buffer size is %d", l, cap(b))
			}
			
			total = append(total, b[:l]...)
			break loop
		case api.SQL_SUCCESS_WITH_INFO:
			err := NewError("SQLGetData", h).(*Error)
			if len(err.Diag) > 0 && err.Diag[0].State != "01004" {
				return nil, err
			}
			i := len(b)
			switch c.CType {
			case api.SQL_C_WCHAR:
				i -= 2 // remove wchar (2 bytes) null-termination character
			case api.SQL_C_CHAR:
				i-- // remove null-termination character
			}
			total = append(total, b[:i]...)
			if l != api.SQL_NO_TOTAL {
				// odbc gives us a hint about remaining data,
				// lets get it in one go.
				n := int(l) // total bytes for our data
				n -= i      // subtract already received
				n += 2      // room for biggest (wchar) null-terminator
				if len(b) < n {
					b = make([]byte, n)
				}
			}
		default:
			return nil, NewError("SQLGetData", h)
		}
	}
	return c.BaseColumn.Value(total)
}
```
在内存不断上涨的环境中跟踪该函数，打印日志发现该函数死循环了，`total = append(total, b[:i]...)`反复执行，导致内存不断上涨。

在有些环境中，内存不上涨，但会返回错误："rows err[SQLGetData: {42000} [FreeTDS][SQL Server]Some character(s) could not be converted into client's character set {01004} [FreeTDS][SQL Server]String data, right truncated]"

可以看出是字符集的问题。SqlServer端使用的是GBK，而客户端配置freeTDS时，使用的是UTF8。有些字符，使用p/ODBC的库，不能够正确的进行字符转换。

只好将客户端的freeTDS字符集也设置为GBK，先从服务器拿到数据，然后程序再对数据做转换。
使用`github.com/axgle/mahonia`库进行字符集的转换。转换代码:
```go
import "github.com/axgle/mahonia"

dec := mahonia.NewDecoder("GBK")
var cerr error
if len(data) > 0 {
	_, ann.AnnTitle, cerr = dec.Translate(data, true)
	if cerr != nil {
		log.Error("GBK Decoder err[%s]", cerr.Error())
	}
}
```

之后，程序就正常工作了。
这应该是`code.google.com/p/odbc`的一个bug，GBK到UTF8转换不能正确进行。

在做查询和并表查询时，也需要注意字符集的问题。
条件允许的话，最好将数据库和客户端的字符集设置成一样的(Unix下中文通常为UTF8)，这样可以减少很多麻烦。

### 其它
* 查询数据库中所有数据库名: `SELECT Name FROM Master..SysDatabases ORDER BY Name`
* 查询数据库中所有的数据表: `SELECT Name FROM SysObjects Where XType='U' ORDER BY Name`
* 查看表结构: `SELECT syscolumns.name, systypes.name, syscolumns.isnullable, syscolumns.length FROM syscolumns, systypes WHERE syscolumns.xusertype = systypes.xusertype  AND syscolumns.id = object_id('表名') `

### 9 go的读取与使用
```go
var c1 sql.NullString, c2 sql.NullFloat64, c3 sql.NullInt64
rows.Scan(&c1, &c2, &c3)
if c1.Valid {
	fmt.Printf("%s", c1.String)
}
if c2.Valid {
	fmt.Printf("%s", c1.Float64)
}
```
