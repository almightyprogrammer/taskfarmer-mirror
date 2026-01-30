import { NextResponse } from "next/server";

export async function PATCH(request) {
  try {
    const body = await request.json();

    const res = await fetch("http://localhost:8080/api/modify", {
      method: "PATCH",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify(body),

      cache: "no-store",
    });

    const text = await res.text();

    return new NextResponse(text, {
      status: res.status,
      headers: {
        "Content-Type": "application/json",
      },
    });
  } catch (err) {
    return NextResponse.json(
      { error: "Proxy modify failed", detail: String(err) },
      { status: 500 }
    );
  }
}