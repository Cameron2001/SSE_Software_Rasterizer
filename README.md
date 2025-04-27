# SSE Software Rasterizer

A tile‑based software renderer using SSE2 intrinsics and C++17 parallel algorithms on Windows.

---

## 🛠 Getting Started

```powershell
git clone https://github.com/your-username/SSE-Software-Rasterizer.git
cd SSE-Software-Rasterizer
python init.py      # generates & installs everything, then opens the VS solution
```

Open `SSESoftwareRasterizer.sln` in Visual Studio 2022, choose Debug or Release, and run.

---

## 🚀 Features

- Quad‑pixel SIMD (`__m128`) rasterization  
- Backface culling via signed‑area test  
- Perspective‑correct attribute interpolation  
- 16×16 tile binning + `std::execution::par` parallelism  
- SIMD‑accelerated Z‑buffer depth test  
- Directional lighting (ambient + Lambertian diffuse)  

---

## 🏗 Architecture & Algorithm

**1. Tile grid**  
&nbsp;&nbsp;Divide the screen into 16×16 pixel tiles.

**2. Triangle binning**  
&nbsp;&nbsp;Assign each triangle to all overlapping tiles by its screen‑space bounding box.

**3. Parallel dispatch**  
&nbsp;&nbsp;Rasterize tiles concurrently with `std::execution::par`.

**4. Scanline pass**  
&nbsp;&nbsp;Within each tile, walk horizontal scanlines to cover candidate pixels.

**5. Edge tests**  
&nbsp;&nbsp;Evaluate half‑space equations (A·x + B·y + C ≥ 0) on 4 pixels at once using SSE2.

**6. Barycentric weights**  
&nbsp;&nbsp;Derive depth, UV and normal interpolation factors from edge values.

**7. Perspective‑correct**  
&nbsp;&nbsp;Interpolate attributes × 1/W, then divide by the interpolated 1/W.

**8. Shading & write‑out**  
&nbsp;&nbsp;Perform SIMD Z‑buffer test, then apply ambient + Lambertian diffuse lighting.

---

## 📷 Screenshot

![Rasterizer Demo](screenshot.png)

---

## 📦 Third‑Party Libraries

- GLM (math)  
- TinyObjLoader (OBJ parsing)  
- stb_image (textures)  
- Conan (package manager)  

---

## 🔗 Resources

- Sam & Max model (Sketchfab):  
  https://sketchfab.com/3d-models/sam-max-7bcb897df5b743909ede7ad9983a5178  
- Intel Intrinsics Guide:  
  https://www.intel.com/content/www/us/en/docs/intrinsics-guide/index.html  
- Rasterization Blog:  
  https://tayfunkayhan.wordpress.com/2019/07/26/chasing-triangles-in-a-tile-based-rasterizer/  
- Rasterization Video:  
  https://www.youtube.com/watch?v=k5wtuKWmV48
- Conan + Premake Template:  
  https://github.com/Ohjurot/ConanPremakeTutorial2  

---

## ⚖️ License

MIT. See [LICENSE](LICENSE).