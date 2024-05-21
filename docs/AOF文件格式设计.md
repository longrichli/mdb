# AOF文件结构设计
魔数：AOF\r\n(5个字节)
版本号：v1.0.0\r\n(8个字节)
数据：数据为命令数组，一个命令分为命令长度和命令内容，命令长度（2个字节，使用uint16_t 表示）， 命令内容（n个字节，n等于命令长度）
# 举例
```txt
AOF\r\nv1.0.0\r\n[14]set\r\nstr1\r\na\r\n[22]lpush\r\nlist\r\na\r\nb\r\nc\r\n
[]中的数字为二进制表示的数据，不是文本数字。
上面表示执行的命令为：
set str1 a
lpush list a b c
```