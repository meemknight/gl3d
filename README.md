# gl3d

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

- [x] Phisically based rendering
- [x] Phisically based materials
- [x] Sky box
- [ ] Image based lighting
- [x] Normal mapping (TBN calculated per vertex)
---
- [x] Deferred rendering
- [ ] Optimize lights 
---
- [x] hdr (16 bit color channel) 
- [ ] automatic exposure ajustment
---
- [ ] FXAA or TAA
- [ ] Adaptive resolution
---
- [ ] Store all the render data before rendering (used for depth pre pass)
- [ ] Improve the deferred rendering by adding bindless things
- [ ] Remove vao for each object
- [ ] Only one geometry buffer and a dynamic index buffer
---
- [x] SSAO
- [ ] SSAO settings
---
- [x] Bloom
- [ ] Bloom settings
---
- [ ] Screen space reflections
---
- [ ] Volumetric light (God rays)
---
- [ ] Directional Shadows
- [ ] Point Shadows
---
- [ ] Transparency




