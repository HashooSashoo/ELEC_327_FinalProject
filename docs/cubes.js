// Cubes distributed along the full page height in the left/right margins.
// The canvas is position:absolute so it covers the whole scrollable document.
// Cubes spin in place — you see different ones as you scroll down.

(function () {
    var CUBE_COUNT = 28;        // total cubes spread top-to-bottom
    var SIDE_FRACTION = 0.16;   // world-unit fraction reserved for each side band

    var s = document.createElement('script');
    s.src = 'https://cdn.jsdelivr.net/npm/three@0.152.2/build/three.min.js';
    s.onload = init;
    document.head.appendChild(s);

    function init() {
        var canvas = document.getElementById('cube-canvas');

        var renderer = new THREE.WebGLRenderer({ canvas: canvas, alpha: true, antialias: true });
        renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        renderer.setClearColor(0x000000, 0);

        var scene = new THREE.Scene();

        // Orthographic camera — we control world coords directly mapped to pixels.
        // We'll update it on resize. Units = pixels.
        var camera = new THREE.OrthographicCamera(0, 1, 1, 0, -1000, 1000);
        camera.position.z = 500;

        // Lighting
        scene.add(new THREE.AmbientLight(0xffffff, 0.55));
        var dir = new THREE.DirectionalLight(0xffffff, 0.9);
        dir.position.set(1, 2, 3);
        scene.add(dir);

        var colors = [0x6c8aff, 0xa78bfa, 0x34d399, 0x60a5fa, 0xf472b6, 0xfbbf24];
        var cubes = [];

        for (var i = 0; i < CUBE_COUNT; i++) {
            var sz = 28 + Math.random() * 32; // size in pixels
            var geo = new THREE.BoxGeometry(sz, sz, sz);
            var mat = new THREE.MeshStandardMaterial({
                color: colors[i % colors.length],
                roughness: 0.35,
                metalness: 0.25,
                transparent: true,
                opacity: 0.5
            });
            var mesh = new THREE.Mesh(geo, mat);

            // Wireframe edges so the cube always reads as 3D
            var edges = new THREE.LineSegments(
                new THREE.EdgesGeometry(geo),
                new THREE.LineBasicMaterial({ color: 0xffffff, transparent: true, opacity: 0.35 })
            );
            mesh.add(edges);

            mesh.rotation.x = Math.random() * Math.PI * 2;
            mesh.rotation.y = Math.random() * Math.PI * 2;
            mesh.userData.rx = (Math.random() - 0.5) * 0.007;
            mesh.userData.ry = (Math.random() - 0.5) * 0.009;
            scene.add(mesh);
            cubes.push(mesh);
        }

        var pageW = 1, pageH = 1; // updated in layout()

        function layout() {
            pageW = document.documentElement.scrollWidth;
            pageH = document.documentElement.scrollHeight;

            // Resize canvas to cover the whole document
            canvas.style.width  = pageW + 'px';
            canvas.style.height = pageH + 'px';
            renderer.setSize(pageW, pageH);

            // Orthographic: origin top-left, x right, y down
            camera.left   = 0;
            camera.right  = pageW;
            camera.top    = 0;
            camera.bottom = pageH;
            camera.updateProjectionMatrix();

            var band = pageW * SIDE_FRACTION; // pixel width of each side column

            cubes.forEach(function (cube, i) {
                var side = (i % 2 === 0) ? 'left' : 'right';
                var x, y;

                if (side === 'left') {
                    x = band * 0.1 + Math.random() * band * 0.8;
                } else {
                    x = pageW - band * 0.1 - Math.random() * band * 0.8;
                }

                // Spread evenly across full page height with some jitter
                var slot = (i / CUBE_COUNT) * pageH;
                y = slot + (Math.random() - 0.5) * (pageH / CUBE_COUNT) * 0.9;
                y = Math.max(30, Math.min(pageH - 30, y));

                // Ortho camera: y axis points DOWN (bottom > top in world coords)
                cube.position.set(x, y, 0);
            });
        }

        // Re-layout when the page finishes rendering (images load, DOM settles)
        function scheduleLayout() {
            // Run immediately and again after a short delay for late-loading content
            layout();
            setTimeout(layout, 600);
            setTimeout(layout, 1800);
        }

        window.addEventListener('resize', scheduleLayout);
        window.addEventListener('load', scheduleLayout);
        scheduleLayout();

        function animate() {
            requestAnimationFrame(animate);
            cubes.forEach(function (c) {
                c.rotation.x += c.userData.rx;
                c.rotation.y += c.userData.ry;
            });
            renderer.render(scene, camera);
        }
        animate();
    }
})();
