# Hard code state space of a player in map. 
# Just a demo, to do this properly, please use a real system instead of a branching story format! 

languages:
en

start: [0, 0]
quit: [quit]


[0, 0]
en:
~~~
+-+-+
| | |
+-+-+
|@| |
+-+-+
~~~

1. [1, 0]
en: right

2. [0, 1]
en: up



[1, 0]
en:
~~~
+-+-+
| | |
+-+-+
| |@|
+-+-+
~~~

1. [0, 0]
en: left

2. [1, 1]
en: up


[0, 1]
en:
~~~
+-+-+
|@| |
+-+-+
| | |
+-+-+
~~~

1. [1, 1]
en: right

2. [0, 0]
en: down


[1, 1]
en:
~~~
+-+-+
| |@|
+-+-+
| | |
+-+-+
~~~

1. [0, 1]
en: left

2. [1, 0]
en: down


