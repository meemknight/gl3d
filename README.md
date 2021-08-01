# gl3d
- Still in working progress (the proper api for the users is not ready)

![](https://github.com/meemknight/photos/blob/master/gl3d1.png)


<h3>How to integrate the library in your project:</h3>

  * From the headerOnly folder add the .h and the .cpp files in your project.
  * You should also have GLM and stb_image and stb_truetype working. You can also find them in dependences.zip.
  * This library uses opengl so you should have the functions loaded. You can do it however you want but you should load them before using the library

---

How to compile the repo (Visual studio 2019, Windows)

<h6>If you want to edit the project this are the steps to follow. It is not required to do this tu use the library on your project</h6>

  * Clone the repo then
  * git submodule init,
  * git submodule update,
  * in the repo directory, unzip dependences,
  * You must have python 3 on your machine for the header only version to be compiled.

---

Features and todos:

- [x] Loading .obj files with materials and textures
- [ ] Loading .mtl files separately
---
- [x] Normal mapping (TBN calculated per fragment)
- [x] Phisically based rendering
- [x] Phisically based materials
- [x] Sky box
- [x] Image based lighting
- [x] Multi light scattering for IBL as described here http://jcgt.org/published/0008/01/03/
- [ ] Screen space reflections
- [ ] Light probes
---
- [x] Deferred rendering
- [ ] Optimize lights 
---
- [x] Gama correction
- [x] HDR (16 bit color channels) 
- [ ] Automatic exposure ajustment for HDR tonemapping
---
- [ ] FXAA or TAA
- [ ] Adaptive resolution
---
- [ ] Store all the render data before rendering (used for depth pre pass)
- [ ] Improve the deferred rendering by adding bindless things
- [ ] Only one geometry buffer and a dynamic index buffer
---
- [x] SSAO
- [x] SSAO settings
---
- [x] Bloom
- [x] Bloom settings
- [x] Emissive materials
---
- [ ] Volumetric light (God rays)
---
- [x] Directional Shadows
- [x] Sofr directional shadows
- [ ] Directional Shadows for multiple lights
- [ ] Directional Shadows settings per light
- [ ] Cascaded Shadow Maps
- [ ] Point Shadows
---
- [ ] Transparency




