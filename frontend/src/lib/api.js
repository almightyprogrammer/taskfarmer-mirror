
const API_BASE = process.env.BACKEND_API_URL;

if (!API_BASE) {
    throw new Error("BACKEND_API_URL is not defined.");
}

export async function fetchChildren(path) {
    const res = await fetch(
    `${API_BASE}/api/ls?path=${encodeURIComponent(path)}`,
    {
        cache: "no-store",
        headers: {
        "X-User-Id": "juwon",
        },
    }
    );

    if (!res.ok) {
    throw new Error(`Backend error: ${res.status}`);
    }

    return res.json();
}