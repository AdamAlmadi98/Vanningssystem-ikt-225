// Enkel linjegraf for jordfuktighet, tegnet på <canvas> uten eksterne bibliotek.

export function drawChart(canvas, points, threshold) {
  const dpr = window.devicePixelRatio || 1;
  const w = canvas.clientWidth;
  const h = canvas.clientHeight;
  canvas.width = w * dpr;
  canvas.height = h * dpr;

  const ctx = canvas.getContext("2d");
  ctx.scale(dpr, dpr);
  ctx.clearRect(0, 0, w, h);

  const css = getComputedStyle(document.documentElement);
  const color = (name) => css.getPropertyValue(name).trim();

  const pad = { l: 36, r: 8, t: 8, b: 22 };
  const pw = w - pad.l - pad.r;
  const ph = h - pad.t - pad.b;
  const y = (v) => pad.t + (1 - v / 100) * ph;

  // Rutenett og y-akse
  ctx.font = "12px system-ui, sans-serif";
  ctx.fillStyle = color("--muted");
  ctx.strokeStyle = color("--border");
  ctx.lineWidth = 1;
  ctx.textAlign = "right";
  ctx.textBaseline = "middle";
  for (const v of [0, 25, 50, 75, 100]) {
    ctx.beginPath();
    ctx.moveTo(pad.l, y(v));
    ctx.lineTo(w - pad.r, y(v));
    ctx.stroke();
    ctx.fillText(`${v}%`, pad.l - 6, y(v));
  }

  const pts = points.filter((p) => p.soil !== null);
  if (pts.length < 2) {
    ctx.textAlign = "left";
    ctx.fillText("Ingen data ennå", pad.l + 10, pad.t + ph / 2);
    return;
  }

  const t0 = pts[0].ts;
  const t1 = pts[pts.length - 1].ts;
  const x = (t) => pad.l + ((t - t0) / (t1 - t0 || 1)) * pw;

  // Vanningsgrense
  if (threshold != null) {
    ctx.save();
    ctx.setLineDash([5, 4]);
    ctx.strokeStyle = color("--warn");
    ctx.beginPath();
    ctx.moveTo(pad.l, y(threshold));
    ctx.lineTo(w - pad.r, y(threshold));
    ctx.stroke();
    ctx.restore();
  }

  // Måledata
  ctx.strokeStyle = color("--accent");
  ctx.lineWidth = 2;
  ctx.lineJoin = "round";
  ctx.beginPath();
  pts.forEach((p, i) => (i ? ctx.lineTo(x(p.ts), y(p.soil)) : ctx.moveTo(x(p.ts), y(p.soil))));
  ctx.stroke();

  // Tidsetiketter
  const fmt = (t) => new Date(t * 1000).toLocaleString("no-NO", { hour: "2-digit", minute: "2-digit" });
  ctx.fillStyle = color("--muted");
  ctx.textBaseline = "top";
  ctx.textAlign = "left";
  ctx.fillText(fmt(t0), pad.l, h - pad.b + 6);
  ctx.textAlign = "right";
  ctx.fillText(fmt(t1), w - pad.r, h - pad.b + 6);
}
