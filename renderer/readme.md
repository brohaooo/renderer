#  An implementation of software renderer

To better understand the traditional render pipeline, I referred to SunXLei's implementation of [SRenderer](https://github.com/SunXLei/SRender), and modified some part of it according to my understanding. This renderer has a flexible camera which can move toward any direction and can load different models with adjustable emission effects. Moreover, I've added a lot of comments to make it easier to understand. (If you want to learn about PBR, please refer to the original author's [SRenderer](https://github.com/SunXLei/SRender))



### Blinn-Phong Shading


<img src="image/mebius.png" width="410" >

<img src="image/triangle.png" width="410">


## Main Features

* render obj models with tga textures
* movable camera
* Blinn-Phong shading
* back-face culling
* homogeneous clipping
* switching of orthogonal and perspective projection
* modifying FOV
* modifying shader's ambient, diffuse and specular ratio



## Binaries for Usage

The pre-built binaries can be found in bin folder. You can start with binaries and obj assets. (Note: the obj folder should be placed in the parent directory of binaries)

## Build

CMakeLists.txt file is provided
Here is an example of building the exe:

For windows 11 with mingw64 compiler and cmake:

under project folder,
```
mkdir build
cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
```
noticing that the exe file should be put in this way:
```
renderer
  ├─build
  │  └─renderer.exe
  ├─obj
  │  └─xxx
```




## Control

### mouse:

* Rotation: holding left mouse button
* Pan: holding right mouse button
* Zoom: mouse wheel(doesn't apply in orthogonal mode)

### keyboard:

* up/down/left/right/forward/backward translation: q/e/a/d/w/s
* camera moving in eye-target mode: 1 (default)
* camera moving in FPS mode: 2
* increase fov: 3
* decrease fov: 4
* perpective projection: 5 (default)
* orthogonal projection: 6
* modify ambient intensity: 7
* modify diffuse intensity: 8
* modify specular intensity: 9
* switch to different scenes: ctrl+1/ctrl+2
* reset position: r

## Reference

[https://github.com/SunXLei/SRender](https://github.com/SunXLei/SRender)

[https://docs.gl/](https://docs.gl/)





