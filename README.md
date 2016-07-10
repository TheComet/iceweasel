Git Stuff
---------
We use  [git  LFS](https://git-lfs.github.com/) for managing large files. Please
download and  install  it,  otherwise  you won't be able to access certain files
(like resources).

You may have to run this command to avoid having to enter your password at every file:
```
git config --global credential.helper cache
```

Building
--------
This  project  depends  on  a   custom   version   of   Urho3D.  Clone  it  from
[https://github.com/TheComet93/Urho3D](https://github.com/TheComet93/Urho3D).   
You will need to checkout the ```iceweasel``` branch.

Install it to a well  known  place  on  your computer (e.g. ```/usr/urho3d``` or
perhaps  ```/home/username/urho3d```  or  ```C:\urho3d``` if you're on Windows).
You        can         set        the        install        location        with
```-DCMAKE_INSTALL_PREFIX=/path/to/installation```.

You will also need to enable the iceweasel mods with ```-DURHO3D_ICEWEASELMODS=ON```

Here are the CMake build options for the settings described above:
```
git checkout iceweasel
cmake -DCMAKE_INSTALL_PREFIX=/usr/urho3d \
      -DURHO3D_LIB_TYPE=SHARED \
      -DURHO3D_ICEWEASELMODS=ON \
      -DURHO3D_SAMPLES=OFF ..
```

Once you've  installed  Urho3D,  go  into the directory ```software/game```. You
will find a script names ```setup-urho3d.sh```. Execute it as:
```
./setup-urho3d.sh /usr/urho3d
```
This will create some symlinks so the Ice Weasel project can find Urho3D.

Next, create a build folder, cd into it and run cmake:
```
mkdir build && cd build
cmake ..
make
```

The binary is placed in the folder ```../bin```.

