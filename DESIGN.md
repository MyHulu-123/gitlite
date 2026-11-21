# Design: $\mathrm{JOHN!}$ $\mathrm{It's}$ $\mathrm{My}$ $\mathrm{Homework!!!!!}$

> SJTU CS1965-01 2026Fall 第三次大作业 设计文档

---

## .gitlite文件夹

```text
.gitlite/
├── objects/                        # 存储所有blob，commit，tree等资产
│   ├── tree......
│   ├── blob......
│   └── commit.....
├── refs/                            # 引用，存放分支、标签以及远程
│   ├── heads/                       # 存放分支
│   │   └──master
│   ├── tags/                        # 存放标签
│   ├── remotes/                     # 存放远程
│   └── remoteDirs/                  # 存放远程仓库路径
└── index                            # 扮演暂存区，记录所有添加的修改，作为下一次commit引用的tree
```

---

## 类的定义

---

#### `Blob`
`Blob`是最基础的类，记录了内容`content`以及其SHA-1ID，通过`getContent()`和`getHash()`获取。Blob创建后不可更改。

#### `Tree`
`Tree`存储了文件名和blobID的映射关系，作为快照，用`vector<Entry>`记录，其中`Entry`是一个含有文件名称和哈希值的结构体。
这个名字来源于PPT中的commit结构，然而就实际情况，改名为`flat`似乎更合适。
提供的函数允许通过文件名查找是否存在对应映射，读取`blobID`，并进行添加和删除。

#### `Commit`
`Commit`存储当前的快照`tree`的ID，时间`time`，两个父提交的ID，以及提交信息`message`。提供的函数可以分别读取这些信息。

由于`Branch`，`tag`和`remote`本质上都只存储地址，所以直接读取，没有设计专门的类来存储。

#### `ObjectDataBase`
一个方法类，提供了许多静态方法，其功能包括：
可以读取`Blob`，`Tree`和`Commit`文件并返回对应的类，同样可以在`.gitlite`内部写入对应文件；
切换分支（远程）指向的提交；
读取当前暂存区、头部提交的内容；
判断分支和标签名称是否合法，等等。
虽然名为`DataBase`，但它实际上没有任何存储数据，其设计意图只是简化读取数据的流程。

#### `Repository`
实现`SomeObj`所有功能（这实际上并不好，但由于初期基本上只是在调用各种其他类的方法，所以就这样做了）。
令人困惑的是，这个类完全可以是静态的，然而我依然要在每次调用时实例化对象。

#### `IgnoreManager`
读取可能存在的`.gitliteignore`，并生成匹配规则，可以传入文件判断是否满足忽视要求。
---
