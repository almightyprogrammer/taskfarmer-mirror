import { NextResponse } from "next/server";

export async function POST(req) {
  const body = await req.json();

  const backendRes = await fetch(
    `${process.env.BACKEND_API_URL}/api/create`,
    {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        "X-User-Id": "juwon",
      },
      body: JSON.stringify(body),
    }
  );

  const text = await backendRes.text();

  return new NextResponse(text, {
    status: backendRes.status,
    headers: { "Content-Type": "application/json" },
  });
}