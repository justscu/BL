# 1. 查看数据表格信息
```sh
# information_schema数据库，存放了整个数据库的基本信息。
mysql> use information_schema;
mysql> desc information_schema.COLUMNS;
+--------------------------+---------------------+------+-----+---------+-------+
| Field                    | Type                | Null | Key | Default | Extra |
+--------------------------+---------------------+------+-----+---------+-------+
| TABLE_CATALOG            | varchar(512)        | NO   |     |         |        |
| TABLE_SCHEMA             | varchar(64)         | NO   |     |         |数据库名|
| TABLE_NAME               | varchar(64)         | NO   |     |         |数据表名|
| COLUMN_NAME              | varchar(64)         | NO   |     |         |表头名  |
| ORDINAL_POSITION         | bigint(21) unsigned | NO   |     | 0       |       |
| COLUMN_DEFAULT           | longtext            | YES  |     | NULL    |缺省值|
| IS_NULLABLE              | varchar(3)          | NO   |     |         |可否为空|
| DATA_TYPE                | varchar(64)         | NO   |     |         |表头类型|
| CHARACTER_MAXIMUM_LENGTH | bigint(21) unsigned | YES  |     | NULL    |       |
| CHARACTER_OCTET_LENGTH   | bigint(21) unsigned | YES  |     | NULL    |       |
| NUMERIC_PRECISION        | bigint(21) unsigned | YES  |     | NULL    |       |
| NUMERIC_SCALE            | bigint(21) unsigned | YES  |     | NULL    |       |
| DATETIME_PRECISION       | bigint(21) unsigned | YES  |     | NULL    |       |
| CHARACTER_SET_NAME       | varchar(32)         | YES  |     | NULL    |       |
| COLLATION_NAME           | varchar(32)         | YES  |     | NULL    |       |
| COLUMN_TYPE              | longtext            | NO   |     | NULL    |表头类型(含大小)|
| COLUMN_KEY               | varchar(3)          | NO   |     |         |主键   |
| EXTRA                    | varchar(27)         | NO   |     |         |       |
| PRIVILEGES               | varchar(80)         | NO   |     |         |       |
| COLUMN_COMMENT           | varchar(1024)       | NO   |     |         |       |
+--------------------------+---------------------+------+-----+---------+-------+

# 查询某个数据表信息的Sql命令
mysql> select * from information_schema.COLUMNS where TABLE_NAME="xxx";
```

# 2. 数据库乱码解决方法
当编码方式不一致时，通常会出现乱码。mysql的字符集细化到4个层次：服务器(server)、数据库(database)、数据表(table)、连接(connection)。使用`status`命令，查看当前mysql的状态。
```sh
(1) 使用命令status
mysql> status;
--------------
mysql  Ver 14.14 Distrib 5.7.12, for Linux (x86_64) using  EditLine wrapper

Connection id:      265166
Current database:   
Current user:       username@10.25.24.37
SSL:            Not in use
Current pager:      stdout
Using outfile:      ''
Using delimiter:    ;
Server version:     5.5.41-MariaDB MariaDB Server
Protocol version:   10
Connection:     10.26.134.181 via TCP/IP
Server characterset:    latin1
Db     characterset:    latin1
Client characterset:    utf8
Conn.  characterset:    utf8
TCP port:       3306
Uptime:         203 days 3 hours 14 min 31 sec


(2) 或者使用命令
mysql> show variables like 'character%';
+--------------------------+----------------------------+
| Variable_name            | Value                      |
+--------------------------+----------------------------+
| character_set_client     | utf8                       | -> 客户端编码方式
| character_set_connection | utf8                       | -> 建立连接使用的编码
| character_set_database   | utf8                       | -> 数据库的编码
| character_set_filesystem | binary                     | -> 
| character_set_results    | utf8                       | -> 结果集的编码
| character_set_server     | latin1                     | -> 数据库服务器的编码
| character_set_system     | utf8                       |
| character_sets_dir       | /usr/share/mysql/charsets/ |
+--------------------------+----------------------------+


(3) 或者查看创建表格时使用的命令
mysql> show create table tbName;


(4) 修改字符集
mysql> alter table tbName charset_set=gbk;
mysql> alter table tbName charset_set=utf8;
```
