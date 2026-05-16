// Rotating cubes and 3D graphics using Three.js
// This script will add animated cubes to the background and as floating elements

// Load Three.js dynamically if not present
(function() {
  if (!window.THREE) {
    var script = document.createElement('script');
    script.src = 'https://cdn.jsdelivr.net/npm/three@0.152.2/build/three.min.js';
    script.onload = initCubes;
    document.head.appendChild(script);
  } else {
    initCubes();
  }

  function initCubes() {
    // Create a canvas overlay for cubes
    const canvas = document.createElement('canvas');
    canvas.id = 'cube-bg-canvas';
    canvas.style.position = 'fixed';
    canvas.style.top = 0;
    canvas.style.left = 0;
    canvas.style.width = '100vw';
    canvas.style.height = '100vh';
    canvas.style.zIndex = 0;
    canvas.style.pointerEvents = 'none';
    document.body.prepend(canvas);

    // Responsive sizing
    function resize() {
      canvas.width = window.innerWidth;
      canvas.height = window.innerHeight;
      renderer.setSize(window.innerWidth, window.innerHeight);
    }

    // Three.js setup
    const renderer = new THREE.WebGLRenderer({ canvas, alpha: true, antialias: true });
    renderer.setClearColor(0x000000, 0); // transparent
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(60, window.innerWidth/window.innerHeight, 0.1, 1000);
    camera.position.z = 8;

    // Add cubes
    const cubes = [];
    const pastelColors = [0xf6c7b6, 0xe0b1cb, 0xe0f7fa, 0xf8e8ee, 0xb2dfdb];
    for (let i = 0; i < 12; i++) {
      const geometry = new THREE.BoxGeometry(1, 1, 1);
      const material = new THREE.MeshStandardMaterial({ color: pastelColors[i % pastelColors.length], roughness: 0.5, metalness: 0.2 });
      const cube = new THREE.Mesh(geometry, material);
      cube.position.x = Math.random() * 16 - 8;
      cube.position.y = Math.random() * 10 - 5;
      cube.position.z = Math.random() * -8;
      cube.rotation.x = Math.random() * Math.PI;
      cube.rotation.y = Math.random() * Math.PI;
      cubes.push(cube);
      scene.add(cube);
    }
    // Lighting
    const ambient = new THREE.AmbientLight(0xffffff, 0.7);
    scene.add(ambient);
    const dir = new THREE.DirectionalLight(0xffffff, 0.5);
    dir.position.set(5, 10, 7);
    scene.add(dir);

    // Animate
    function animate() {
      requestAnimationFrame(animate);
      cubes.forEach((cube, i) => {
        cube.rotation.x += 0.003 + i * 0.0005;
        cube.rotation.y += 0.004 + i * 0.0003;
      });
      renderer.render(scene, camera);
    }
    animate();
    window.addEventListener('resize', resize);
    resize();
  }
})();
