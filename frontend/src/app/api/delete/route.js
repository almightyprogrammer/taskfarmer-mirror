import { NextResponse } from "next/server";

export async function DELETE(req) {
  let body;

  try {
    body = await req.json();
  } catch {
    return NextResponse.json(
      { error: "Invalid JSON body" },
      { status: 400 }
    );
  }

  if (!body.id || typeof body.id !== "string") {
    return NextResponse.json(
      { error: "Missing or invalid field: id" },
      { status: 400 }
    );
  }

  const backendRes = await fetch(
    `${process.env.BACKEND_API_URL}/api/delete`,
    {
      method: "DELETE",
      headers: {
        "Content-Type": "application/json",
        "X-User-Id": "juwon",
      },
      body: JSON.stringify({ id: body.id }),
      cache: "no-store",
    }
  );

  const text = await backendRes.text();

  return new NextResponse(text, {
    status: backendRes.status,
    headers: { "Content-Type": "application/json" },
  });
}