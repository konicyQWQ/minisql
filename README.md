# minisql

## catalog manager

### 文件说明

一个 tbf.inf 存储所有表名，一个 idx.inf 存储所有索引

一个文件存储一个表的所有信息，存储格式如下。

```
xxx.tbf 对应 xxx表
--------------------
byte     byte    byte
字段数量  索引数量 主键是第几个属性？
80*byte  byte    byte             byte                byte
属性名称  类型     char(n)类型中的n  是否为unique(1表示是)  是否不能为NULL(1表示不能为NULL)
         0:char
         1:int
         2:float
80*byte  byte
索引名称  索引第几个属性？
```

一个文件最大是：$84*32+81*32+3=5283 Byte$

### 错误信息
+ createTable 返回 1 表示表重名, 0 表示成功创建
+ dropTable   返回 1 表示表不存在，0 表示成功删除
+ createIndex 返回 1 表示表不存在，2 表示索引已经重复过了，0 表示成功创建
+ dropIndex   返回 1 表示索引不存在，0表示删除成功

## api

首先，通过catalog获取表的相关信息，然后：
+ 创建表：catalog创建表（错误信息由这个控制），record创建表，index创建索引
+ 删除：catalog删除（错误信息由这个控制），record删除表，index删除索引
+ 创建索引：catalog创建索引，index创建索引
+ 删除索引：catalog删除索引，index删除索引
+ insert：
+ delete：
+ select：

## buffer manager

`Block* getBlock(std::string filename, int offset);`

这个是返回 filename 这个文件的第 offset 块的指针，offset取值是0，1，2，3...，filename对应的这个必须存在。`Block->buf[x]`就是对应缓冲区块的第 x 个字节，一块是 4096 个字节。当 offset=0 时，这个 buf 就是文件第 0~4095字节，后面同理。

blk->validChar 是一个 int 类型变量，表示这4096个字节里面，有多少个是有效的字节（也就是说，如果文件只有4096个字节，用offset=1读取一个本没有的块的时候，这个值就是0）

`void writeBlock(Block* blk);`

当你要对这块缓冲区进行写操作的时候，需要用这个函数标记一下。

`void pinBlock(Block *blk);`

当你不希望这块缓冲区被后面的缓冲区顶掉的时候，可以用这个函数锁住它。

`void removeBlock(Block* blk);`

强制将 blk 这块缓冲区写回文件。不管他有没有被锁住。