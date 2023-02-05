# ==== Overview ==== 

# Any line that starts with '#' is a comment, and will be ignored by the interpreter, as if there is no line.

# Anything that looks like [foo] is used to indicate a scene label, including start label and quit label.

# Anything that looks like this is a (localized) text field:
# en: yes
# zh: 好的

# A text field must be ended with a blank line.
# A text field does not need to have all languages, and the missing language will be displayed as "{missing string}".



# ==== Header ==== 

# A story file must have a language list like this.
# The language list must be ended with a blank line.
# The first language will be the default language of the story.
languages:
en
zh
jp


# A story file must have a start label and a quit label.
# The start label is the scene you will be started at. You need to define it yourself.
# The quit label is to provide a exit game function as an option in a scene. You don't need to define it yourself, since it's the exit.
# You can use utf-8 text for label names.
start: [start]
quit:  [quit]



# ==== Story Scenes ==== 

# ---- A scene definition ----

# This is the start label. 
[start]
# A scene must have a text field. If you don't write anything but a blank line here, the interpreter will just think all the strings are missing. 
en: A Basic Story
zh: 初级故事
jp: 初級物語

# A scene need to have options. (For now, we won't report error if a scene label has no option.) 
# This is an option. the number in "1." does not matter. 
# The [choose] part is which scene this option will lead to.
1. [choose]
# An option need to have a text field. When the text field ends (with a blank line), the option is also ended. 
en: start game
zh: 开始游戏

# Another option. Notice this is the quit label, so if you choose this you will quit the game.
2. [quit]
en: quit game
zh: 退出游戏


# A scene definition must be ended with a blank line. 
#  
# Since a scene must be ended with a blank line, and an option must be ended with a blank line too, 
# this means any scene must be ended with "2 blank lines". 
#  
# Also because of this, you can't use more than 1 line to separate the text field of a scene and the options. 
#
# This is not by design and is just the result of the parser being not smart, 
# but I think it is actually easier to read this way, so this is the syntax for now.



# ---- Another scene definition ----

[choose]
# You can use a pair of "~~~" to use multi-line text for a text field, but only in scene text fields, for now. 
en: 
~~~
=======================
What do you want to do?
=======================
~~~
zh: 
~~~
===========
你想怎么做？
===========
~~~

# You can use utf-8 text for label names.
1. [赢]
en: try to win
zh: 试着赢

# It's legal to write it like this, the interpreter will just pick up the number automatically.
# But it's probably easier to read if you use the correct number for it.  
0. [lose]
en: try to lose
zh: 试着输


[赢]
en: You win!
zh: 你赢了！

1. [start]
en: start game again
zh: 重新开始游戏

2. [quit]
en: quit game
zh: 退出游戏


[lose]
en: You lose!
zh: 你输了！

1. [start]
en: start game again
zh: 重新开始游戏

2. [quit]
en: quit game
zh: 退出游戏


