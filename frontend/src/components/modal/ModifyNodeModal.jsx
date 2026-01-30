"use client";

import { useState } from "react";
import "./modal.css";

export default function ModifyNodeModal({ node, onCancel, onSave }) {
  const [title, setTitle] = useState(node.title);
  const [description, setDescription] = useState(node.description || "");
  const [status, setStatus] = useState(node.status);
  const [priority, setPriority] = useState(node.priority);
  const [submitting, setSubmitting] = useState(false); 

  async function submit() {
    if (submitting) return; 
    setSubmitting(true);

    try {
      await onSave({
        id: node.id,
        title,
        description,
        status,
        priority,
      });
    } finally {

      setSubmitting(false);
    }
  }

  return (
    <div className="modal-backdrop">
      <div className="modal-window">
        <div className="modal-title">Modify Task</div>

        <div className="modal-content">
          <label>
            Title
            <input
              value={title}
              onChange={(e) => setTitle(e.target.value)}
              disabled={submitting}
            />
          </label>

          <label>
            Description
            <textarea
              rows={3}
              value={description}
              onChange={(e) => setDescription(e.target.value)}
              disabled={submitting}
            />
          </label>

          <label>
            Status
            <select
              value={status}
              onChange={(e) => setStatus(Number(e.target.value))}
              disabled={submitting}
            >
              <option value={0}>Open</option>
              <option value={1}>In Progress</option>
              <option value={2}>Done</option>
            </select>
          </label>

          <label>
            Priority
            <select
              value={priority}
              onChange={(e) => setPriority(Number(e.target.value))}
              disabled={submitting}
            >
              <option value={0}>Low</option>
              <option value={1}>Medium</option>
              <option value={2}>High</option>
              <option value={3}>Critical</option>
            </select>
          </label>
        </div>

        <div className="modal-buttons">
          <button disabled={submitting} onClick={submit}>
            {submitting ? "Saving..." : "OK"}
          </button>
          <button disabled={submitting} onClick={onCancel}>
            Cancel
          </button>
        </div>
      </div>
    </div>
  );
}