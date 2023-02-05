# Windows + MinGW
name="story"
src="src/main.c"
opt="-O0"
etc="-std=c99 -pedantic -Wall -Wextra"

# build
gcc $src $opt $etc -o bin/$name &&

# run
cd bin && ./$name run "data/test.story" && cd ..

