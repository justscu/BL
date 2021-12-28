.
|-- README.md
|-- global
|   |-- dym1
|   |   |-- CMakeLists.txt
|   |   |-- debug
|   |   |-- dym1.cpp
|   |   `-- dym1.h
|   |-- dym2
|   |   |-- CMakeLists.txt
|   |   |-- dym2.cpp
|   |   `-- dym2.h
|   |-- exe
|   |   |-- CMakeLists.txt
|   |   `-- main.cpp
|   `-- staticx
|       |-- CMakeLists.txt
|       |-- staticx.cpp
|       `-- staticx.h
`-- static
    |-- dym1
    |   |-- CMakeLists.txt
    |   |-- dym1.cpp
    |   `-- dym1.h
    |-- dym2
    |   |-- CMakeLists.txt
    |   |-- dym2.cpp
    |   `-- dym2.h
    |-- exe
    |   |-- CMakeLists.txt
    |   `-- main.cpp
    `-- staticx
        |-- CMakeLists.txt
        |-- staticx.cpp
        `-- staticx.h

global中使用全局变量的方式
```
ctor() this[0x7fb9b2ccb060]
ctor() this[0x7fb9b2ccb060]
data[1]
data[2]
dtor() this[0x7fb9b2ccb060]
dtor() this[0x7fb9b2ccb060]
```

static使用static局部变量的方式
```
ctor() this[0x7fcee4a8c090]
data[1]
data[2]
dtor() this[0x7fcee4a8c090]
```
