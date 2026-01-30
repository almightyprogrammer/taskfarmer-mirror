export const dynamic = "force-dynamic";

import Explorer from "@/components/explorer/Explorer";
import { fetchChildrenServer } from "@/lib/serverApi";

export default async function TaskExplorerPage() {
  
  const rootNodes = await fetchChildrenServer("ROOT");

  return (
    <div style={{ height: "100vh" }}>
      <Explorer
        initialNodes={rootNodes}
        initialBackendPath="ROOT"
        initialUiPath="ROOT"
      />
    </div>
  );
}