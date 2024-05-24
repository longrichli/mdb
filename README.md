# mdb
## mdb 介绍
一个仿照redis实现的一个内存数据库
## 主要功能
1.  提供五种数据类型的存取：string，hash，list，set，sorted set
2.	持久化功能

## 实现命令
### string
set key value
get key
append key value
incrby key increment            x
decrby key decrement            x
incr key                        x
strlen key
setrange key
getrange key
### list
lpush key val [val...]
rpush key val [val...]
lpop key
llen key
lindex key idx
linsert key [after/before] val newval
lrem key count val
ltrim key start end
lset key idx val
lrange key start end
### hash
hset key k v [k v ...]
hget key k
hexists key k
hdel key k
hlen key
hkeys key
hgetall key
### set
sadd key member1 member2 ...
scard key
sismember key member
smembers key
srandmember key
spop key count
srem key member1 member2 ...
### zset


### common
select idx
keys pattern
del key
type key
exists key
rename key newkey      x
renamex key newkey     x







