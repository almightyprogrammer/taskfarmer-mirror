"use client";

import "./modal.css";

export default function DeleteConfirmModal({ onCancel, onConfirm }) {
  return (
    <div className="modal-backdrop">
      <div className="modal-window">
        <div className="modal-title">Confirm Delete</div>

        <div className="modal-content">
          <p>
            This operation is irreversible.
            <br />
            All data will be permanently lost.
          </p>
        </div>

        <div className="modal-buttons">
          <button onClick={onConfirm}>Delete</button>
          <button onClick={onCancel}>Cancel</button>
        </div>
      </div>
    </div>
  );
}