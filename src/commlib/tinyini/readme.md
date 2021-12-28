间单的ini格式配置文件

注释支持`#`&`;`

包含 `Section`和`key-value`两种数据

`Section`: 以`[ ]`包含；必须含有Section. 如“[section]”

`key-value`: 如“key1=value”, “key2=value2;”

```
[SHL1B]
key1 =.e value#;
key2 = value; #
key3 = S #value


[SHL2]
key1 = x,x,v,y,w


[SZ]
uri1=10.10.25.26:9988
```
