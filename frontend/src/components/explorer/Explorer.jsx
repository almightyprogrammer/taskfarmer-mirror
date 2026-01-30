"use client";

import { useState } from "react";
import ExplorerToolbar from "./ExplorerToolbar";
import Tree from "@/components/tree/Tree";
import Navbar from "@/components/navbar/Navbar";
import CreateNodeModal from "@/components/modal/CreateNodeModal";
import LoginModal from "@/components/modal/LoginModal";
import { createNode, fetchChildrenClient } from "@/lib/clientApi";
import {
  childrenCache,
  hasChildrenCache,
} from "@/components/tree/treeCache";

export default function Explorer({
  initialNodes,
  initialBackendPath,
}) {
  const [historyBack, setHistoryBack] = useState([]);
  const [historyForward, setHistoryForward] = useState([]);
  const [selectedPath, setSelectedPath] = useState(null);

  const [showCreateModal, setShowCreateModal] = useState(false);
  const [showLoginModal, setShowLoginModal] = useState(false);

  const [current, setCurrent] = useState({
    backendPath: initialBackendPath, 
    uiPath: ["ROOT"],  
    nodes: initialNodes,
  });

  function navigate(next) {
    setHistoryBack((prev) => [...prev, current]);
    setHistoryForward([]);

    setCurrent({
      backendPath: next.backendPath,
      uiPath: [...current.uiPath, next.title],
      nodes: next.nodes,
    });

    setSelectedPath(null);
  }

  function goBack() {
    if (!historyBack.length) return;

    const prev = historyBack[historyBack.length - 1];
    setHistoryBack((h) => h.slice(0, -1));
    setHistoryForward((f) => [current, ...f]);
    setCurrent(prev);
    setSelectedPath(null);
  }

  function goForward() {
    if (!historyForward.length) return;

    const next = historyForward[0];
    setHistoryForward((f) => f.slice(1));
    setHistoryBack((h) => [...h, current]);
    setCurrent(next);
    setSelectedPath(null);
  }

  async function handleCreate(data) {
    setShowCreateModal(false);

    const payload = {
      parent_id: current.backendPath,
      title: data.title,
      description: data.description,
      priority: data.priority,
      status: data.status,
    };


    await createNode(payload);

  
    const refreshed = await fetchChildrenClient(current.backendPath);


    childrenCache.set(current.backendPath, refreshed);
    hasChildrenCache.set(current.backendPath, refreshed.length > 0);


    setCurrent((prev) => ({
      ...prev,
      nodes: refreshed,
    }));
  }

  function handleDelete(id) {
    setCurrent((prev) => ({
      ...prev,
      nodes: prev.nodes.filter((n) => n.id !== id),
    }));
  }

  return (
    <div style={{ height: "100%", display: "flex", flexDirection: "column" }}>
      <Navbar onLogin={() => setShowLoginModal(true)} />

      <ExplorerToolbar
        onBack={goBack}
        onForward={goForward}
        path={current.uiPath}
        canBack={historyBack.length > 0}
        canForward={historyForward.length > 0}
        onCreate={() => setShowCreateModal(true)}
      />

      <div style={{ flex: 1, minHeight: 0 }}>
        <Tree
          nodes={current.nodes}
          selectedPath={selectedPath}
          setSelectedPath={setSelectedPath}
          onNavigate={navigate}
          onDelete={handleDelete}
        />
      </div>

      {showCreateModal && (
        <CreateNodeModal
          onCancel={() => setShowCreateModal(false)}
          onCreate={handleCreate}
        />
      )}

      {showLoginModal && (
        <LoginModal onCancel={() => setShowLoginModal(false)} />
      )}
    </div>
  );
}