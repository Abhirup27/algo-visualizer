import { Editor } from "@monaco-editor/react";
const EditorOptions = {
  automaticLayout: true,
  contextmenu: true,
  dragAndDrop: true,
  dropIntoEditor: {
    enabled: true,
  },
  detectIndentation: false,
  fixedOverflowWidgets: false,
  fontFamily: "Fira Mono",
  fontSize: 16,
  formatOnPaste: true,
  formatOnType: true,
  lineHeight: 1.5,
  minimap: {
    enabled: true,
  },
  padding: {
    top: 8,
    button: 8,
  },
  scrollbar: {
    verticalScrollbarSize: 9,
    horizontalScrollbarSize: 9,
    alwaysConsumeMouseWheel: false,
  },
  scrollBeyondLastLine: false,
  smoothScrolling: true,
  tabSize: 4,
  quickSuggestions: true,
  // wordBasedSuggestions: true,
};

export default function MonacoEditor({
  code,
  onChange,
  readOnly,
}: {
  code: string;
  onChange: (value: string) => void;
  readOnly?: boolean;
}) {
  return (
    <>
      <Editor
        options={{ ...EditorOptions, readOnly: false }}
        value={code}
        theme="vs-dark"
        language="python"
        onChange={(value) => onChange(value ?? "")}
      />
    </>
  );
}

