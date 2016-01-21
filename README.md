# batchtool
Tool for trimming and merging png textures into a spritesheet.

#### Building
To build batchtool, clone it first:
```
git clone git@github.com:n0va/batchtool.git batchtool
```

The libraries are added as submodules, so doing this will download all dependencies:
```
cd batchtool
git submodule init
git submodule update
```

Then, just build using cmake:
```
mkdir build
cd build
cmake ..
make
```

To test out if everything is working, use the test images from res/:
```
./batchtool -vtb -o test -f json ../res/*
```
This will output test.png and test.json into the executable directory.
