"use client";

import { useState } from "react";
import "./modal.css";

export default function CreateNodeModal({ onCancel, onCreate }) {
  const [title, setTitle] = useState("");
  const [description, setDescription] = useState("");
  const [priority, setPriority] = useState(1);
  const [status, setStatus] = useState(0);

  function submit() {
    if (!title.trim()) return;
    onCreate({
      title,
      description,
      priority: Number(priority),
      status: Number(status),
    });
  }

  return (
    <div className="modal-backdrop">
      <div className="modal-window">
        <div className="modal-title">Create New Task</div>

        <div className="modal-content">
          <label>
            Title
            <input
              autoFocus
              value={title}
              onChange={(e) => setTitle(e.target.value)}
            />
          </label>

          <label>
            Description
            <textarea
              rows={3}
              value={description}
              onChange={(e) => setDescription(e.target.value)}
            />
          </label>

          <label>
            Priority
            <select
              value={priority}
              onChange={(e) => setPriority(e.target.value)}
            >
              <option value={0}>Low</option>
              <option value={1}>Medium</option>
              <option value={2}>High</option>
              <option value={3}>Critical</option>
            </select>
          </label>

          <label>
            Status
            <select
              value={status}
              onChange={(e) => setStatus(e.target.value)}
            >
              <option value={0}>Open</option>
              <option value={1}>In Progress</option>
              <option value={2}>Done</option>
            </select>
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