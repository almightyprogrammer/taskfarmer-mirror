"use client";

import "./navbar.css";

export default function Navbar({ onLogin }) {
  return (
    <div className="win98-navbar">
      <div className="win98-navbar-title">
        <img
          src="/directory.png"
          alt=""
          className="win98-title-icon"
        />
        <span className="win98-title-text">taskfarmer</span>
      </div>

      <div className="win98-navbar-spacer" />

      <button onClick={onLogin}>Sign In</button>
    </div>
  );
}