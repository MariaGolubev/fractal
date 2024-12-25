# FRACTAL

<div style="margin: 0; display: flex; justify-content: center; align-items: center; position: relative;">
  <div style="width: 50vw; overflow-x: scroll; scroll-snap-type: x mandatory; display: flex; scrollbar-width: none;">
    <section style="flex: 0 0 100%; height: 100%; display: flex; justify-content: center; align-items: center; scroll-snap-align: start; scroll-snap-stop: always;">
      <img src="Screenshot1.png" alt="Screenshot1" style="object-fit: cover;">
    </section>
    <section style="flex: 0 0 100%; height: 100%; display: flex; justify-content: center; align-items: center; scroll-snap-align: start; scroll-snap-stop: always;">
      <img src="Screenshot2.png" alt="Screenshot2" style="object-fit: cover;">
    </section>
    <section style="flex: 0 0 100%; height: 100%; display: flex; justify-content: center; align-items: center; scroll-snap-align: start; scroll-snap-stop: always;">
      <img src="Screenshot3.png" alt="Screenshot3" style="object-fit: cover;">
    </section>
  </div>
  <div style="position: absolute; left: 10px; top: 50%; transform: translateY(-50%);">
    <span style="font-size: 20px;">&lt;</span>
  </div>
  <div style="position: absolute; right: 10px; top: 50%; transform: translateY(-50%);">
    <span style="font-size: 20px;">&gt;</span>
  </div>
</div>

A libadwaita application that draws multiple Mandelbrot sets in a GLSL shader. The code needs to be rewritten, but the process of writing this application was fun