export async function fetchChildrenClient(parentId) {
  const res = await fetch(`/api/ls?parent_id=${parentId}`, {
    cache: "no-store",
  });

  if (!res.ok) {
    throw new Error("Failed to fetch children");
  }

  return res.json();
}

export async function createNode(payload) {
  const res = await fetch("/api/create", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });

  if (!res.ok) {
    throw new Error("Failed to create node");
  }

  return res.json();
}

export async function modifyNode(payload) {
  const res = await fetch("/api/modify", {
    method: "PATCH",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payload),
  });

  if (!res.ok) {
    throw new Error("Failed to modify node");
  }

  return res.json();
}