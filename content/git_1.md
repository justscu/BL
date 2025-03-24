## git源码管理工具

git是一个分布式的源码管理系统，每个客户端上都有一份完整的代码，不依赖中央管理库。

[git官方网址](http://git-scm.com/about)

[git中文教程](http://www.liaoxuefeng.com/wiki/0013739516305929606dd18361248578c67b8067c8c017b000)

### 1 安装
sudo apt-get install git

### 2 修改用户名
```sh
(1)修改全局配置 vim ~/.git/.gitconfig，添加
[user]
    name=justscu
    email=justscu@163.com
    
(2)也可以对某个工程进行单独修改，vim projectname/.git/.gitconfig，添加
[user]
    name=justscu
    email=justscu@163.com
```

### 3 Github token

`Settings` -> `Developer settings` -> `Persional access tokens` -> `click "Generate new token"` 可所有的框都选，点击`Generate token` 
-> 把生成的token拷贝到./git/config文件 : `url = https://生成的tocken@github.com/justscu/BL.git`


### 4 基本命令
- 下载：`git clone https://github.com/justscu/tracestack.git  tracestack.git`
- 不带SSH认证下载：env GIT_SSL_NO_VERIFY=true git clone https://github.com/justscu/tracestack.git  tracestack.git
- 只clone某个分支：`git clone v2 https://github.com/go-mgo/mgo.git`, 可能有很多分支，但只clone其v2分支 
- 查看远端库地址：`git remote -v`
- 查看有哪些文件：`git ls-files`
- 添加新文件：`git add xx.cpp xx.h`
- 提交文件到本地：`git commit -m "提交更新" xx.cpp xx.h`
- 提交文件到服务器：`git push`
- 查看日志：`git log`，`git log --pretty=oneline`
- 查看本地和服务器的不同：`git diff`
- 本地更新：`git pull`
- 显示有哪些文件：`git ls-files`
- 显示有哪些文件被删掉了：`git ls-files --deleted`
- 恢复被删掉的文件env.rc：`git checkout -- env.rc`

### 5 分支管理
- 查看有哪些分支：`git branch --all`
- 创建分支：`git branch branch_name`
- 切换分支：`git checkout branch_name`
- 创建+切换分支：`git branch -b branch_name`
- 推送到新分支：`git push origin feature/tbt:feature/tbt`，远端分支名为feature/tbt
- 合并某分支到当前分支：`git merge branch_name`, 合并branch_name到当前分支
- 删除本地分支：`git branch -d branch_name`
- 删除远程分支：`git push origin 空格:远程分支`, 将一个空push到远程分支
- 查看分支状态：`git status`
- 只提交到某个分支：`git push origin feature/newsinfo`，feature/newsinfo为分支
- 建立本地分支与远程分支的关联: `git branch --set-upstream branch-name origin/branch-name`
- 删除(某个)远程分支的本地引用: `git branch -rd origin/feature/fix_decode_bug`
- 删除(所有)远程分支的本地引用: `git remote prune origin`
