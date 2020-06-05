# catalog manager

## 文件说明

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

## 错误信息
createTable 返回 1 表示表重名, 0 表示成功创建
dropTable   返回 1 表示表不存在，0 表示成功删除
createIndex 返回 1 表示表不存在，2 表示索引已经重复过了，0 表示成功创建
dropIndex   返回 1 表示索引不存在，0表示删除成功