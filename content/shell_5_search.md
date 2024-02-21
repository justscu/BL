
### google 搜索框

[google高级搜索](https://www.google.com/advanced_search)

AND和OR必须大写，否则被认为是普通单词。所有指令和符号 都是英文状态，冒号后不能有空格

指令和符号 | 说明 
----------:|------
 AND | 与，如`A AND B`表示A和B都要有
  OR | 或，如`A OR B`表示A或B满足一个即可. OR的优先级高于AND
  \- | 非，如`-B`表示排除B
  \* | 通配符，表示一连串字符. 如`biggest * in the world`
   ? | 通配符，表示单个字符. 
  .. | 数值范围，`手机 3000..6000 元`，表示[3000, 6000]元的手机，3000和6000前后有空格
  \~ | 近义词，如`~laptop`
  "" | 双引号，完全匹配精确搜索，如`"biggest pen in the world"`, 把引号中的内容当作一个词进行搜索
intitle      | 在网站标题中查找，`熊猫 野外 intitle:陕西`，标题中必须含有“陕西”，内容中含有“熊猫”和“野外”
allintitle   | `大熊猫 OR 小熊猫 allintitle:陕西 四川`
inurl        | `留学费用 inurl:edu` 表示url中含有edu 
allinurl     | `allinurl:gov.cn links`表示url中同时含有gov.cn和links
site         | site表示域名
inanchor     | 
allinanchor  |
intext       | 正文
allintext    |
insubject    | 主题
allinsubject |
filetype     | 文件格式如`chat-gpt filetype:ppt`表示搜索ppt格式的chat-gpt
