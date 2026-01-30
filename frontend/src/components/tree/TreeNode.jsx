"use client";

import { useState } from "react";
import { fetchChildrenClient, modifyNode } from "@/lib/clientApi";
import ModifyNodeModal from "@/components/modal/ModifyNodeModal";
import DeleteConfirmModal from "@/components/modal/DeleteConfirmModal";
import ContextMenu from "@/components/contextmenu/ContextMenu";
import { childrenCache, hasChildrenCache } from "./treeCache";


function priorityClass(priority) {
  switch (priority) {
    case 0: return "priority-low";
    case 1: return "priority-medium";
    case 2: return "priority-high";
    case 3: return "priority-critical";
    default: return "";
  }
}

function statusLabel(status) {
  switch (status) {
    case 0: return "Open";
    case 1: return "In Progress";
    case 2: return "Done";
    case 3: return "Blocked";
    default: return "";
  }
}

function formatTime(ts) {
  if (ts == null) return "—";
  return new Date(ts * 1000).toLocaleString();
}

function nodeIcon(hasChildren, expanded) {
  if (!hasChildren) return "📄";
  if (expanded) return "📂";
  return "📁";
}


async function eagerLoadTwoLevels(parentId) {
  let level1;

  if (childrenCache.has(parentId)) {
    level1 = childrenCache.get(parentId);
  } else {
    level1 = await fetchChildrenClient(parentId);
    childrenCache.set(parentId, level1);
    hasChildrenCache.set(parentId, level1.length > 0);
  }

  for (const child of level1) {
    const childId = child.id;
    if (childrenCache.has(childId)) continue;

    const level2 = await fetchChildrenClient(childId);
    childrenCache.set(childId, level2);
    hasChildrenCache.set(childId, level2.length > 0);
  }
}


export default function TreeNode({
  node,
  selectedPath,
  setSelectedPath,
  onNavigate,
  onDelete,
}) {
  const [expanded, setExpanded] = useState(false);
  const [children, setChildren] = useState(
    childrenCache.get(node.id) ?? null
  );

  const [localNode, setLocalNode] = useState(node);

  const [showModify, setShowModify] = useState(false);
  const [showDelete, setShowDelete] = useState(false);
  const [contextMenu, setContextMenu] = useState(null);

  const hasChildren = hasChildrenCache.get(node.id) ?? false;
  const isSelected = selectedPath === node.id;

  async function handleClick() {
    setSelectedPath(node.id);

    if (!expanded) {
      await eagerLoadTwoLevels(node.id);
      setChildren(childrenCache.get(node.id) ?? []);
    }

    setExpanded(!expanded);
  }

  async function handleDoubleClick() {
    await eagerLoadTwoLevels(node.id);

    onNavigate({
      backendPath: node.id,
      title: localNode.title,
      nodes: childrenCache.get(node.id),
    });
  }

  async function saveChanges(updated) {
    setShowModify(false);
    await modifyNode(updated);

    setLocalNode((prev) => ({
      ...prev,
      ...updated,
    }));
  }

  async function confirmDelete() {
    setShowDelete(false);

    await fetch("/api/delete", {
      method: "DELETE",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ id: node.id }),
    });

    const stack = [node.id];
    while (stack.length) {
      const id = stack.pop();
      const kids = childrenCache.get(id) || [];
      kids.forEach((k) => stack.push(k.id));
      childrenCache.delete(id);
      hasChildrenCache.delete(id);
    }

    onDelete(node.id);
  }

  return (
    <li
      onContextMenu={(e) => {
        e.preventDefault();
        setContextMenu({ x: e.clientX, y: e.clientY });
      }}
    >
      <div
        className={`tree-node ${priorityClass(localNode.priority)} ${
          isSelected ? "selected" : ""
        }`}
        onClick={handleClick}
        onDoubleClick={handleDoubleClick}
      >
        <span className="tree-node-icon">
          {nodeIcon(hasChildren, expanded)}
        </span>

        <div className="tree-node-text">
          <div className="tree-node-title">{localNode.title}</div>

          <div className="tree-node-meta">
            <span className="meta-status">
              {statusLabel(localNode.status)}
            </span>
            <span className="meta-dot">•</span>
            <span className={`meta-priority ${priorityClass(localNode.priority)}`}>
              {["Low", "Medium", "High", "Critical"][localNode.priority]}
            </span>
            <span className="meta-dot">•</span>
            <span className="meta-time">
              Updated {formatTime(localNode.last_updated_at)}
            </span>
          </div>
        </div>

        {localNode.status === 2 && (
          <span
            style={{
              marginLeft: "12px",
              display: "flex",
              alignItems: "center",
            }}
          >
            <img
              src="/green_tick.png"
              alt="Completed"
              style={{ width: "14px", height: "14px" }}
            />
          </span>
        )}
      </div>

      {expanded && (
        <div className="tree-children">
          {localNode.description && (
            <div className="tree-node-description">
              {localNode.description}
            </div>
          )}

          {children && children.length > 0 && (
            <ul>
              {children.map((child) => (
                <TreeNode
                  key={child.id}
                  node={child}
                  selectedPath={selectedPath}
                  setSelectedPath={setSelectedPath}
                  onNavigate={onNavigate}
                  onDelete={onDelete}
                />
              ))}
            </ul>
          )}
        </div>
      )}

      {contextMenu && (
        <ContextMenu
          x={contextMenu.x}
          y={contextMenu.y}
          onModify={() => {
            setContextMenu(null);
            setShowModify(true);
          }}
          onDelete={() => {
            setContextMenu(null);
            setShowDelete(true);
          }}
          onClose={() => setContextMenu(null)}
        />
      )}

      {showModify && (
        <ModifyNodeModal
          node={localNode}
          onCancel={() => setShowModify(false)}
          onSave={saveChanges}
        />
      )}

      {showDelete && (
        <DeleteConfirmModal
          onCancel={() => setShowDelete(false)}
          onConfirm={confirmDelete}
        />
      )}
    </li>
  );
}