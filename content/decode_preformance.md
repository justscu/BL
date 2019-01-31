### SHL2

> split STEP

FileSize(Byte)|  Packages| ns/package| Byte/us|
-------------:|---------:|----------:|-------:|
 1,284,675,942| 7,256,876|        351|   14259|


> decode STEP

 type|AvgSize(B)|CheckSum(ns)|decode(us)|
-----|---------:|-----------:|---------:|
   md|     13950|        1106|     45.21
  opt|      3066|         406|     27.75
  tbt|      9434|         874|     20.09

> decode FAST

 type|AvgSize(B)|decode(ns)|
----:|---------:|---------:|
   md|       105|       341
  opt|       160|      1608
  tbt|        28|        60


### SZL2
> split

FileSize(Byte)|   Packages| ns/packages| Byte/us|
-------------:|----------:|-----------:|-------:|
 2,480,169,420| 10,027,692|          15|   16000


> decode

 type|AvgSize(B)|CheckSum(ns)|decode(ns)
----:|---------:|-----------:|---------:|
   md|      1067|          85|       182
index|       131|          22|        49
  tbt|        70|          20|        11

### interface

multicast cost

  pem(%)| 25%| 50%| 75%| 90%|100%|interface
-------:|---:|---:|---:|---:|---:|--------:|
cost(us)| 2.0| 2.1| 2.4| 2.5| 2.6| 10G card
cost(us)| 2.1| 2.6| 2.7| 2.8| 3.5|  1G card
