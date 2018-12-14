# 模拟生成随机数

### 文件说明

`gen_hash` 本地计算hash的可执行文件，由`gen_hash.cpp` 编译生成。hash计算算法与`random.cpp`合约文件的算法一致。

`gen_hash.cpp`本地计算hash的源文件与`random.cpp`合约文件的算法一致。

`gen_random_log.log`记录 历史命令

`gen_random.py` 重要文件。包含编译生成`gen_hash` 生成合约abi,  wasm， 创建部署合约的帐号，部署合约， 控制三个帐号 `pushhash` `pushvalue`，获取三个帐号每轮的原始值，根据原始值计算随机数。

`random.abi` 合约文件

`random.cpp`合约文件。有两个action，`pushhash` `pushvalue`，分别将原始值的hash，原始值上传

`random.txt`生成的随机数写入该文件

`random.wasm`合约文件

`random.wast`合约文件

### 用法

要求先用`./bios-boot-tutorial.py -a`配置好运行环境。之后会用到配置好的几个帐号`useraaaaaaaa` `useraaaaaaab` `useraaaaaaac` 

在当前路径下执行`./gen_random.py -r 100` `100`表示要生成的随机数个数，三秒生成一个。

### 说明

当前参与生成随机数的帐号是由合约指定。

生成间隔可在`gen_random.py`中调整。



### TODO

1. 创建一个全局状态表，记录当前生成随机数有哪些帐号，进行到第几轮，上一轮成功在第几轮，记录没有提交值的帐号。

2. 对比较旧的随机数能自动删除，避免占用ram。

3. 更换hash算法。

4. 

### 命令参考

```
./cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000 set contract useruseruser ./../random/

./cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000 push action useruseruser pushhash '["useraaaaaaaa", 0, "36016129175642747"]' -p useraaaaaaaa

./cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000 push action useruseruser pushvalue '["useraaaaaaaa", 0, "64233267"]' -p useraaaaaaaa

./cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000 get table useruseruser 1 randoms
```