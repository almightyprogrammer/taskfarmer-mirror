"use client";

export default function ExplorerToolbar({
  onBack,
  onForward,
  path,
  canBack,
  canForward,
  onCreate,
}) {
  return (
    <div className="win98-toolbar">
      <button disabled={!canBack} onClick={onBack}>←</button>
      <button disabled={!canForward} onClick={onForward}>→</button>

      <button onClick={onCreate}>New</button>

      <div className="win98-path">
        {path.join("\\")}
      </div>
    </div>
  );
}