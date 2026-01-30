"use client";

import { useEffect, useState } from "react";
import TreeNode from "./TreeNode";
import { fetchChildrenClient } from "@/lib/clientApi";
import { childrenCache, hasChildrenCache } from "./treeCache";
import "./tree.css";
import "../modal/spinner.css";

export default function Tree({
  nodes,
  selectedPath,
  setSelectedPath,
  onNavigate,
  onDelete,
}) {
  const [ready, setReady] = useState(false);

  useEffect(() => {
    let cancelled = false;

    async function eagerLoadTwoLevels() {
      for (const node of nodes) {
        const parentId = node.id;

        try {
          const level1 = await fetchChildrenClient(parentId);
          if (cancelled) return;

          childrenCache.set(parentId, level1);
          hasChildrenCache.set(parentId, level1.length > 0);

          for (const child of level1) {
            const childId = child.id;
            if (childrenCache.has(childId)) continue;

            const level2 = await fetchChildrenClient(childId);
            if (!cancelled) {
              childrenCache.set(childId, level2);
              hasChildrenCache.set(childId, level2.length > 0);
            }
          }
        } catch {}
      }

      if (!cancelled) setReady(true);
    }

    eagerLoadTwoLevels();
    return () => { cancelled = true; };
  }, [nodes]);

  if (!ready) {
    return (
      <div className="spinner-overlay">
        <div className="spinner" />
      </div>
    );
  }

  return (
    <div className="tree-container">
      <ul className="tree-root">
        {nodes.map((node) => (
          <TreeNode
            key={node.id}
            node={node}
            parentId={node.id}
            selectedPath={selectedPath}
            setSelectedPath={setSelectedPath}
            onNavigate={onNavigate}
            onDelete={onDelete}
          />
        ))}
      </ul>
    </div>
  );
}