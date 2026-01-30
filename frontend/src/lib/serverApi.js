const API_BASE = process.env.BACKEND_API_URL;

if (!API_BASE) {
  throw new Error("BACKEND_API_URL is not defined.");
}

export async function fetchChildrenServer(parentId) {
  const res = await fetch(
    `${process.env.BACKEND_API_URL}/api/ls?parent_id=${parentId}`,
    {
      headers: {
        "X-User-Id": "juwon",
      },
      cache: "no-store",
    }
  );

  if (!res.ok) {
    throw new Error(`Backend error: ${res.status}`);
  }

  return res.json();
}