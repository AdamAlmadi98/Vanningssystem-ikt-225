// Alle kall mot serverens REST-API samlet på ett sted.

async function request(path, options = {}) {
  const res = await fetch(`/api${path}`, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });
  if (!res.ok) {
    let message = res.statusText;
    try {
      const detail = (await res.json()).detail;
      message = Array.isArray(detail) ? detail.map((d) => d.msg).join(", ") : detail || message;
    } catch { /* ignorer */ }
    throw new Error(message);
  }
  return res.json();
}

const post = (path, body) => request(path, { method: "POST", body: JSON.stringify(body ?? {}) });

export const api = {
  status: () => request("/status"),
  history: (hours) => request(`/history?hours=${hours}`),
  events: (limit = 30) => request(`/events?limit=${limit}`),
  water: (seconds) => post("/water", { seconds }),
  stop: () => post("/stop"),
  config: (cfg) => post("/config", cfg),
};
