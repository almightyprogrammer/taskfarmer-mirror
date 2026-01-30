"use client";

export default function ContextMenu({ x, y, onModify, onDelete, onClose }) {
  return (
    <div
      style={{
        position: "fixed",
        top: y,
        left: x,
        background: "#c0c0c0",
        border: "2px solid",
        borderColor: "#ffffff #404040 #404040 #ffffff",
        fontFamily: "MS Sans Serif, Tahoma, sans-serif",
        fontSize: "12px",
        zIndex: 2000,
        minWidth: "120px",
      }}
      onClick={onClose}
    >
      <div
        style={{ padding: "4px", cursor: "default" }}
        onClick={(e) => {
          e.stopPropagation();
          onModify();
        }}
      >
        Modify
      </div>

      <div
        style={{ padding: "4px", cursor: "default" }}
        onClick={(e) => {
          e.stopPropagation();
          onDelete();
        }}
      >
        Delete
      </div>
    </div>
  );
}