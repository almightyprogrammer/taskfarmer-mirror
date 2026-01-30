"use client";

import { useState } from "react";
import "./modal.css";

export default function LoginModal({ onCancel }) {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");

  function submit() {
    if (!username.trim() || !password.trim()) return;

    alert(`Signed in as ${username}`);
    onCancel();
  }

  return (
    <div className="modal-backdrop">
      <div className="modal-window">
        <div className="modal-title">Sign In</div>

        <div className="modal-content">
          <label>
            Username
            <input
              autoFocus
              value={username}
              onChange={(e) => setUsername(e.target.value)}
            />
          </label>

          <label>
            Password
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
            />
          </label>
        </div>

        <div className="modal-buttons">
          <button onClick={submit}>OK</button>
          <button onClick={onCancel}>Cancel</button>
        </div>
      </div>
    </div>
  );
}