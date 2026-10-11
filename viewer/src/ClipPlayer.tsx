import { useEffect, useRef, useState } from "react";

export function ClipPlayer({ src }: { src: string }) {
  const videoRef = useRef<HTMLVideoElement>(null);
  const [rate, setRate] = useState(1);

  function changeRate(value: number) {
    const video = videoRef.current;
    if (!video) return;
    video.playbackRate = Math.max(0.25, Math.min(4, value));
    setRate(video.playbackRate);
  }

  function seek(seconds: number) {
    const video = videoRef.current;
    if (!video || !Number.isFinite(video.duration)) return;
    // currentTime is media time: never multiply the jump by playbackRate.
    video.currentTime = Math.max(0, Math.min(video.duration, video.currentTime + seconds));
  }

  useEffect(() => {
    function onKey(event: KeyboardEvent) {
      if (event.altKey || event.ctrlKey || event.metaKey || event.shiftKey) return;
      const target = event.target;
      if (target instanceof HTMLElement &&
          (target.isContentEditable || target.closest('input, select, textarea, [role="textbox"]'))) return;
      if (!["ArrowRight", "ArrowLeft", "ArrowUp", "ArrowDown"].includes(event.key)) return;
      event.preventDefault();
      event.stopImmediatePropagation(); // Override native video seek/volume arrow shortcuts.
      if (event.key === "ArrowRight") changeRate((videoRef.current?.playbackRate ?? 1) + 0.25);
      if (event.key === "ArrowLeft") changeRate((videoRef.current?.playbackRate ?? 1) - 0.25);
      if (event.key === "ArrowUp") seek(15);
      if (event.key === "ArrowDown") seek(-15);
    }
    window.addEventListener("keydown", onKey, true);
    return () => window.removeEventListener("keydown", onKey, true);
  }, []);

  return <>
    <video ref={videoRef} className="clip-player" src={src} controls preload="metadata"
      aria-label="Recorded gameplay" onRateChange={e => setRate(e.currentTarget.playbackRate)} />
    <div className="playback-controls" role="group" aria-label="Clip playback controls">
      <button onClick={() => seek(-15)} aria-keyshortcuts="ArrowDown">↓ −15s</button>
      <button onClick={() => changeRate(rate - 0.25)} disabled={rate <= 0.25}
        aria-label="Decrease playback speed" aria-keyshortcuts="ArrowLeft">← Slower</button>
      <output aria-live="polite" aria-label="Playback speed">{rate}×</output>
      <button onClick={() => changeRate(rate + 0.25)} disabled={rate >= 4}
        aria-label="Increase playback speed" aria-keyshortcuts="ArrowRight">Faster →</button>
      <button onClick={() => seek(15)} aria-keyshortcuts="ArrowUp">+15s ↑</button>
      <div className="speed-presets" role="group" aria-label="Speed presets">
        {[1, 1.5, 2, 3, 4].map(value => <button key={value}
          aria-pressed={rate === value} onClick={() => changeRate(value)}>{value}×</button>)}
      </div>
    </div>
    <p className="clip-note">← / → speed ±0.25× · ↓ / ↑ back / forward 15 video seconds</p>
  </>;
}
