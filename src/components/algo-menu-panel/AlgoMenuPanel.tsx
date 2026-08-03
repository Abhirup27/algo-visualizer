import { useEffect, useMemo, useState, type ReactNode, type Ref, type RefObject } from "react";
import { SceneTypeLabel } from "../../types/scene_registry";
import type { WasmModule } from "../../types/wasmmodule";

export interface AlgoDescriptor {
  id: string;
  key: string;
  name: string;
  category: string;
  subCategories: string;
}

export const LOAD_ALGORITHM_EVENT = "load_algorithm";

const SCENE_TYPE: Record<string, string> = {
  "0": "Graph / Tree",
  "1": "Sort",
};

export default function AlgoMenuPanel({ ref, wasmModule }: { ref: Ref<HTMLElement>, wasmModule: RefObject<WasmModule> }) {
  const [algorithms, setAlgorithms] = useState<AlgoDescriptor[]>([]);
  const [groupVisible, setGroupVisibility] = useState<Record<string, boolean>>(
    {},
  );
  const [selectedKey, setSelectedKey] = useState<string | null>(null);

  useEffect(() => {
    const handler = (e: Event) => {
      setAlgorithms((e as CustomEvent<AlgoDescriptor[]>).detail);
    };
    window.addEventListener("register_algorithms", handler, { once: true });
    return () => window.removeEventListener("register_algorithms", handler);
  }, []);

  const algoGroups = useMemo(() => {
    const map = new Map<string, AlgoDescriptor[]>();
    for (const algo of algorithms) {
      if (!map.has(algo.category)) map.set(algo.category, []);
      map.get(algo.category)!.push(algo);
    }
    return map;
  }, [algorithms]);

  const toggleVisibility = (category: string) => {
    setGroupVisibility((prev) => ({ ...prev, [category]: !prev[category] }));
  };

  return (
    <aside ref={ref} className="sidebar panel">
      <div className="title">
        <h3>Algorithm List</h3>
      </div>
      <div className="panel-subsection">
        <div id="algo-list">
          {Array.from(algoGroups.entries()).map<ReactNode>(
            ([category, algos]) => (
              <div key={category}>
                <button
                  type="button"
                  className="dropdown-btn"
                  onClick={() => toggleVisibility(category)}
                >
                  {SceneTypeLabel[category] ?? `Category ${category}`}
                  <svg
                    className={`arrow ${!!groupVisible[category] ? "open" : ""}`}
                    width="16"
                    height="10"
                    fill="currentColor"
                    viewBox="0 0 8 5"
                  >
                    <path d="M 0 0 L 4 5 L 8 0" />
                  </svg>
                </button>
                <ul
                  className={`dropdown-li ${!!groupVisible[category] ? "open" : ""}`}
                >
                  {algos.map((algo) => (
                    <li key={algo.id}>
                      <button
                        className={`dropdown-li-btn ${selectedKey === algo.key ? "selected" : ""}`}
                        onClick={() => {
                          setSelectedKey(algo.key);
                          wasmModule.current._set_algorithm(algo.id);
                          window.dispatchEvent(
                            new CustomEvent<AlgoDescriptor>(
                              LOAD_ALGORITHM_EVENT,
                              { detail: algo },
                            ),
                          );
                        }}
                      >
                        {algo.name}
                      </button>
                    </li>
                  ))}
                </ul>
              </div>
            ),
          )}
        </div>
      </div>
    </aside>
  );
}

