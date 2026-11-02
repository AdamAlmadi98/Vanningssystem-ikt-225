import { api } from "./api.js";
import { drawChart } from "./chart.js";

const $ = (id) => document.getElementById(id);
const POLL_MS = 5000;

let lastStatus = null;
let configLoaded = false;
const notifiedAlerts = new Set();

// ---------- Hjelpefunksjoner ----------

function toast(text) {
  const el = $("toast");
  el.textContent = text;
  el.hidden = false;
  clearTimeout(toast.timer);
  toast.timer = setTimeout(() => (el.hidden = true), 3000);
}

function setCard(id, text, cls = "") {
  $(id).textContent = text;
  const card = $(id).closest(".card");
  card.classList.remove("bad", "warn", "active");
  if (cls) card.classList.add(cls);
}

const REASONS = { manual: "manuelt", auto: "automatisk", schedule: "tidsplan", user: "stoppet av bruker" };
const PROBLEMS = { low_water: "lavt vannivå", water_sensor_fault: "feil på vannivåsensor" };

function describeEvent(e) {
  const d = e.data || {};
  switch (e.type) {
    case "watered":     return `Vannet ${(d.ms / 1000).toFixed(1)} s (${REASONS[d.reason] || d.reason})`;
    case "safety_stop": return `Sikkerhetsstopp: ${PROBLEMS[d.stopped] || d.stopped}`;
    case "rejected":    return `Vanning avvist: ${PROBLEMS[d.reason] || d.reason}`;
    case "low_water":   return "Lavt vannivå i tanken";
    case "needs_water": return `Planten trenger vann (${d.soil} %)`;
    case "schedule_skipped": return `Tidsplan hoppet over – jorden er våt (${d.soil} %)`;
    case "boot":        return "ESP32 startet";
    default:            return e.type;
  }
}

// ---------- Visning ----------

function renderStatus(st) {
  lastStatus = st;
  const d = st.data;

  const conn = $("conn");
  conn.textContent = st.online ? "Tilkoblet" : "Offline";
  conn.className = `badge ${st.online ? "online" : "offline"}`;

  if (d) {
    const dry = d.soil_ok && d.soil < d.threshold;
    setCard("soil", d.soil_ok ? `${d.soil} %` : "Feil", !d.soil_ok ? "bad" : dry ? "warn" : "");
    $("temp").textContent = d.aht_ok ? `${d.temp.toFixed(1)} °C` : "–";
    $("hum").textContent = d.aht_ok ? `${Math.round(d.hum)} %` : "–";
    setCard("water", !d.water_sensor_ok ? "Feil" : d.water_ok ? "OK" : "Lavt", d.water_ok && d.water_sensor_ok ? "" : "bad");
    setCard("pump", d.pump ? "Går" : "Av", d.pump ? "active" : "");
    $("count").textContent = d.water_count;

    if (!configLoaded) {
      $("cfg-auto").checked = d.auto;
      $("cfg-threshold").value = d.threshold;
      $("cfg-interval").value = String(d.interval_h);
      $("cfg-seconds").value = d.seconds;
      $("water-seconds").value = d.seconds;
      configLoaded = true;
    }
  }

  $("alerts").innerHTML = "";
  for (const a of st.alerts) {
    const div = document.createElement("div");
    div.className = `alert ${a.level}`;
    div.textContent = a.text;
    $("alerts").append(div);
    notify(a.text);
  }
  // Glem varsler som er borte, så de kan varsles igjen senere
  for (const text of [...notifiedAlerts]) {
    if (!st.alerts.some((a) => a.text === text)) notifiedAlerts.delete(text);
  }
}

function renderEvents(events) {
  $("events").innerHTML = "";
  if (!events.length) {
    $("events").innerHTML = "<li>Ingen hendelser ennå</li>";
    return;
  }
  for (const e of events) {
    const li = document.createElement("li");
    const time = new Date(e.ts * 1000).toLocaleString("no-NO", {
      day: "2-digit", month: "2-digit", hour: "2-digit", minute: "2-digit",
    });
    li.innerHTML = `<span></span><time>${time}</time>`;
    li.firstChild.textContent = describeEvent(e);
    $("events").append(li);
  }
}

async function renderChart() {
  const points = await api.history($("hist-hours").value);
  drawChart($("chart"), points, lastStatus?.data?.threshold);
}

// ---------- Varsler i nettleseren ----------

function notify(text) {
  if (notifiedAlerts.has(text)) return;
  notifiedAlerts.add(text);
  if ("Notification" in window && Notification.permission === "granted") {
    new Notification("Vanningssystem", { body: text });
  }
}

// ---------- Oppdatering ----------

async function refresh() {
  try {
    renderStatus(await api.status());
    renderEvents(await api.events());
    await renderChart();
  } catch (err) {
    $("conn").textContent = "Server utilgjengelig";
    $("conn").className = "badge offline";
  }
}

// ---------- Knapper og skjema ----------

async function action(fn, okText) {
  try {
    await fn();
    toast(okText);
    setTimeout(refresh, 1000);
  } catch (err) {
    toast(`Feil: ${err.message}`);
  }
}

$("btn-water").addEventListener("click", () =>
  action(() => api.water(Number($("water-seconds").value)), "Vanning startet"));

$("btn-stop").addEventListener("click", () => action(api.stop, "Stopp sendt"));

$("config-form").addEventListener("submit", (e) => {
  e.preventDefault();
  action(() => api.config({
    auto: $("cfg-auto").checked,
    threshold: Number($("cfg-threshold").value),
    interval_h: Number($("cfg-interval").value),
    seconds: Number($("cfg-seconds").value),
  }), "Innstillinger lagret");
});

$("hist-hours").addEventListener("change", renderChart);
window.addEventListener("resize", renderChart);

$("btn-notify").addEventListener("click", async () => {
  if (!("Notification" in window)) return toast("Nettleseren støtter ikke varsler");
  const perm = await Notification.requestPermission();
  toast(perm === "granted" ? "Varsler er slått på" : "Varsler ble ikke tillatt");
});

refresh();
setInterval(refresh, POLL_MS);
