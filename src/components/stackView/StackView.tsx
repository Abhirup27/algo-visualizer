import { useEffect, useRef, useState, type RefObject } from "react";
import { EventActionLabel, EventTargetLabel } from "../../types/events";
import type { MainModule } from "../../types/wasmmodule";
import type { GenericScriptStackFrame } from "../../types/InfoPanel";
import type { EventDescriptor } from "../../types/EventDescriptor";

export default function StackView({
  items,
  onUpdate,
  wasmModule,
}: {
  items: GenericScriptStackFrame[];
  onUpdate: (stack: Array<GenericScriptStackFrame>) => void;
  wasmModule: RefObject<MainModule>;
}) {
  const prevItems: RefObject<GenericScriptStackFrame[]> = useRef<GenericScriptStackFrame[]>([]);
  const [newItems, SetNewItems] = useState<GenericScriptStackFrame[]>([]);
  useEffect(() => {
    const handler = (e: CustomEvent) => {
      const event: EventDescriptor = e.detail;
      console.log(event.action, event.target);
      if (EventActionLabel[event.action] === EventActionLabel[3]) {
        if (EventTargetLabel[event.target] === EventTargetLabel[3]) {

          const ptr = wasmModule.current._get_script_stack_json();
          const json = wasmModule.current.UTF8ToString(ptr);
          const stack: GenericScriptStackFrame[] = JSON.parse(json);
          console.log(stack);
          onUpdate(stack);
        }
      }
    };

    window.addEventListener("scene_event", handler as EventListener);
    return () =>
      window.removeEventListener("scene_event", handler as EventListener);
  }, []);
  useEffect(() => {
    const prevSet = new Set(prevItems.current.map((f) => `${f.node}`));
    const newlyAdded = items.filter((item) => !prevSet.has(`${item.node}`));

    if (newlyAdded.length > 0) {
      SetNewItems(newlyAdded);
    }
    prevItems.current = items;
  }, [items]);

  return (
    <>
      <div className="stack-view">
        {items.map((item, index) => (
          <p
            key={index}
            className={
              +newItems.includes(item)
                ? "stack-cell stack-new-element"
                : "stack-cell stack-element"
            }
          >
            {item.node}
          </p>
        ))}
      </div>
    </>
  );
}
