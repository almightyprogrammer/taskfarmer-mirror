import { NextResponse } from "next/server";

export async function GET(req) {
  const { searchParams } = new URL(req.url);
  const parentId = searchParams.get("parent_id");

  if (!parentId) {
    return new Response(
      JSON.stringify({ error: "missing parent_id" }),
      { status: 400 }
    );
  }

  const backendRes = await fetch(
    `${process.env.BACKEND_API_URL}/api/ls?parent_id=${parentId}`,
    {
      headers: {
        "X-User-Id": "juwon",
      },
      cache: "no-store",
    }
  );

  const text = await backendRes.text();

  return new Response(text, {
    status: backendRes.status,
    headers: { "Content-Type": "application/json" },
  });
}