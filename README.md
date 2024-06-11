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
incrby key increment
decrby key decrement
incr key
strlen key
setrange key offset value
getrange key start end
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
HEXSITS key k
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
zadd key score1 member1 score2 member2 ...
zcard key
zcount key min max
zrange key start stop
zrevrange key start stop
zrank key member
zrevrank key member
zrem key member1 member2
zsore key member
### common
select idx
keys pattern
del key
type key
exists key
rename key newkey
renamex key newkey







