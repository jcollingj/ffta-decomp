import { useEffect, useRef, useState } from "react";
import "./index.css";
import { ClipPlayer } from "./ClipPlayer";

type Row = { ts: string; tag: string; msg: string; key: number };
type Clip = { name: string; seconds: number; url: string };

const LINE = /^(\d\d:\d\d:\d\d)\s+\[(\w+)\]\s+(.*)$/;

function parseLog(text: string): Row[] {
  const rows = text
    .split("\n")
    .filter(l => l.trim())
    .map((l, i) => {
      const m = l.match(LINE);
      return m
        ? { ts: m[1], tag: m[2], msg: m[3], key: i }
        : { ts: "", tag: "log", msg: l, key: i };
    });
  // Preserve append order across midnight and multiple play sessions.
  return rows;
}

const KNOWN = new Set(["luna", "play", "emu", "shot", "session"]);

const MILESTONE =
  /battle|victor|defeat|won|title menu|new game|name|transform|ivalice|save|complete|joined|outcome/i;

function tsToMs(ts: string, ref: number): number {
  const [h, m, s] = ts.split(":").map(Number);
  const d = new Date(ref);
  d.setHours(h, m, s, 0);
  let t = d.getTime();
  if (t > ref + 60_000) t -= 86_400_000; // logged before midnight
  return t;
}

function Timeline({ rows, now }: { rows: Row[]; now: number }) {
  const stamped = rows.filter(r => r.ts);
  if (stamped.length < 2) return null;
  const t0 = tsToMs(stamped[0].ts, now);
  const span = Math.max(now - t0, 60_000);
  const pct = (t: number) => ((t - t0) / span) * 100;

  const milestones = stamped.filter(
    r => r.tag === "session" || (r.tag !== "shot" && MILESTONE.test(r.msg)),
  );
  // thin out milestone labels: keep last of any cluster within 2.5% of each other
  const labeled: Row[] = [];
  for (const m of milestones) {
    const last = labeled[labeled.length - 1];
    if (last && pct(tsToMs(m.ts, now)) - pct(tsToMs(last.ts, now)) < 2.5) labeled.pop();
    labeled.push(m);
  }

  // axis ticks: 5 evenly spaced
  const ticks = Array.from({ length: 5 }, (_, i) => {
    const t = t0 + (span * i) / 4;
    const d = new Date(t);
    const hh = String(d.getHours()).padStart(2, "0");
    const mm = String(d.getMinutes()).padStart(2, "0");
    return { left: (i / 4) * 100, label: `${hh}:${mm}` };
  });

  return (
    <div className="timeline">
      <div className="tl-track">
        {stamped.map(r => (
          <span
            key={r.key}
            className={`tl-dot t-${KNOWN.has(r.tag) ? r.tag : "other"}`}
            style={{ left: `${pct(tsToMs(r.ts, now))}%` }}
            title={`${r.ts} [${r.tag}] ${r.msg}`}
          />
        ))}
        {labeled.map(m => (
          <span
            key={`m${m.key}`}
            className="tl-mark"
            style={{ left: `${pct(tsToMs(m.ts, now))}%` }}
            title={`${m.ts} — ${m.msg}`}
          >
            <span className="tl-flag" />
            <span className="tl-label">{m.msg.slice(0, 26)}{m.msg.length > 26 ? "…" : ""}</span>
          </span>
        ))}
        <span className="tl-now" style={{ left: "100%" }} />
      </div>
      <div className="tl-axis">
        {ticks.map((t, i) => (
          <span key={i} className="tl-tick" style={{ left: `${t.left}%` }}>{t.label}</span>
        ))}
      </div>
    </div>
  );
}

export function App() {
  const [frameSrc, setFrameSrc] = useState("");
  const [rows, setRows] = useState<Row[]>([]);
  const [live, setLive] = useState(false);
  const [clips, setClips] = useState<Clip[]>([]);
  const [selectedClip, setSelectedClip] = useState("");
  const [now, setNow] = useState(() => Date.now());
  const feedRef = useRef<HTMLDivElement>(null);
  const pinned = useRef(true);

  useEffect(() => {
    const poll = async () => {
      try {
        const response = await fetch('/api/clips');
        if (response.ok) setClips(await response.json());
      } catch {}
    };
    void poll();
    const timer = setInterval(poll, 5000);
    return () => clearInterval(timer);
  }, []);

  useEffect(() => {
    const id = setInterval(() => setNow(Date.now()), 1000);
    return () => clearInterval(id);
  }, []);

  useEffect(() => {
    let stop = false;
    let prevUrl = "";
    (async () => {
      while (!stop) {
        try {
          const r = await fetch("/api/screen");
          if (r.ok) {
            const url = URL.createObjectURL(await r.blob());
            setFrameSrc(url);
            setLive(true);
            if (prevUrl) URL.revokeObjectURL(prevUrl);
            prevUrl = url;
          } else setLive(false);
        } catch {
          setLive(false);
        }
        await new Promise(res => setTimeout(res, 150));
      }
    })();
    return () => { stop = true; };
  }, []);

  useEffect(() => {
    const el = feedRef.current;
    const onScroll = () => {
      if (!el) return;
      pinned.current = el.scrollHeight - el.scrollTop - el.clientHeight < 60;
    };
    el?.addEventListener("scroll", onScroll);
    const id = setInterval(async () => {
      try {
        const r = await fetch("/api/log");
        setRows(parseLog(await r.text()));
        requestAnimationFrame(() => {
          if (pinned.current) el?.scrollTo({ top: el.scrollHeight });
        });
      } catch {}
    }, 800);
    return () => { clearInterval(id); el?.removeEventListener("scroll", onScroll); };
  }, []);

  const lastTs = [...rows].reverse().find(r => r.ts)?.ts;
  let ago = "";
  let staleness = "fresh";
  if (lastTs) {
    const [h, m, s] = lastTs.split(":").map(Number);
    const d = new Date(now);
    d.setHours(h, m, s, 0);
    let secs = Math.floor((now - d.getTime()) / 1000);
    if (secs < 0) secs += 86400; // log line from just before midnight
    ago =
      secs < 60 ? `${secs}s ago`
      : secs < 3600 ? `${Math.floor(secs / 60)}m ${secs % 60}s ago`
      : `${Math.floor(secs / 3600)}h ${Math.floor((secs % 3600) / 60)}m ago`;
    staleness = secs > 300 ? "stale" : secs > 120 ? "aging" : "fresh";
  }

  return (
    <div className="shell">
      <header className="hdr">
        <h1>FFTA Tactical Ops</h1>
        <nav className="app-nav" aria-label="Main navigation"><a href="/" aria-current="page">Game & clips</a><a href="/guide">Decomp guide</a></nav>
        <span className="sub">CMD: codex&nbsp;&nbsp;·&nbsp;&nbsp;PILOT: luna</span>
        <div className={`live ${live ? "on" : "off"}`}>
          <span className="dot" />
          {live ? "live" : "offline"}
        </div>
      </header>

      <Timeline rows={rows} now={now} />

      <main className="main">
        <section className="stage">
          {selectedClip ? (
            <ClipPlayer key={selectedClip} src={selectedClip} />
          ) : frameSrc ? (
            <div className="crt"><img src={frameSrc} alt="game screen" /></div>
          ) : (
            <p className="waiting">waiting for emulator…</p>
          )}
          <div className="clip-picker">
            <label htmlFor="clip-select">Watch </label>
            <select id="clip-select" value={selectedClip} onChange={e => setSelectedClip(e.target.value)}>
              <option value="">Live game</option>
              {clips.map(clip => <option key={clip.name} value={clip.url}>
                {clip.name.replaceAll('-', ' ')} · {Math.round(clip.seconds)}s
              </option>)}
            </select>
            {selectedClip && <a href={selectedClip} download>Download MP4</a>}
          </div>
          {selectedClip && <p className="clip-note">Silent clip · game time · thinking pauses omitted</p>}
        </section>

        <section className="logpanel">
          <div className="cap">
            <span className="lead">▮</span> operation log
            {ago && <span className={`ago ${staleness}`}>last action {ago}</span>}
          </div>
          <div className="feed" ref={feedRef}>
            {rows.length === 0 && <div className="empty">no log yet…</div>}
            {rows.map((r, i) => (
              <div
                key={r.key}
                className={`row t-${KNOWN.has(r.tag) ? r.tag : "other"} ${i === rows.length - 1 ? "latest" : ""}`}
              >
                {r.ts && <span className="ts">{r.ts}</span>}
                <span className="tag">{r.tag}</span>
                <span className="msg">{r.msg}</span>
              </div>
            ))}
          </div>
        </section>
      </main>
    </div>
  );
}

export default App;
