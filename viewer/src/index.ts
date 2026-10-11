import { serve } from "bun";
import index from "./index.html";

const EMU = "http://localhost:8080";
const LOG_PATH = new URL("../../logs/playthrough.log", import.meta.url).pathname;
const CLIPS_PATH = new URL("../../logs/clips/", import.meta.url).pathname;

const server = serve({
  port: 3000,
  routes: {
    "/*": index,

    "/api/clips": async () => {
      const clips = [];
      try {
        for await (const path of new Bun.Glob("*/recording.json").scan(CLIPS_PATH)) {
          const meta = await Bun.file(`${CLIPS_PATH}${path}`).json();
          if (meta.finished_utc && !meta.incomplete && !meta.aborted_utc) {
            clips.push({ name: meta.name, seconds: meta.video_frames / meta.fps,
              finished: meta.finished_utc, url: `/clips/${meta.name}` });
          }
        }
      } catch { /* No recordings yet. */ }
      clips.sort((a, b) => b.finished.localeCompare(a.finished));
      return Response.json(clips, { headers: { "Cache-Control": "no-store" } });
    },

    "/clips/:name": async req => {
      const name = req.params.name;
      if (!/^[a-z0-9_-]+$/.test(name)) return new Response(null, { status: 400 });
      const file = Bun.file(`${CLIPS_PATH}${name}/clip.mp4`);
      if (!await file.exists()) return new Response(null, { status: 404 });
      const headers = { "Content-Type": "video/mp4", "Accept-Ranges": "bytes" };
      const range = req.headers.get("Range");
      if (range) {
        const match = /^bytes=(\d+)-(\d*)$/.exec(range);
        if (!match) return new Response(null, { status: 416 });
        const start = Number(match[1]);
        const end = match[2] ? Math.min(Number(match[2]), file.size - 1) : file.size - 1;
        if (start > end || start >= file.size) return new Response(null, { status: 416 });
        return new Response(file.slice(start, end + 1), { status: 206,
          headers: { ...headers, "Content-Range": `bytes ${start}-${end}/${file.size}`,
            "Content-Length": String(end - start + 1) } });
      }
      return new Response(file, { headers });
    },

    "/api/screen": async () => {
      try {
        const r = await fetch(`${EMU}/screen`, { signal: AbortSignal.timeout(2000) });
        return new Response(await r.arrayBuffer(), {
          headers: { "Content-Type": "image/png", "Cache-Control": "no-store" },
        });
      } catch {
        return new Response(null, { status: 503 });
      }
    },

    "/api/log": async () => {
      const f = Bun.file(LOG_PATH);
      const text = (await f.exists()) ? await f.text() : "";
      const lines = text.trimEnd().split("\n").slice(-300).join("\n");
      return new Response(lines, {
        headers: { "Content-Type": "text/plain", "Cache-Control": "no-store" },
      });
    },
  },

  development: process.env.NODE_ENV !== "production" && { hmr: true, console: true },
});

console.log(`🎮 FFTA viewer at ${server.url}`);
