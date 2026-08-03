// vite-plugin-wasm-types.ts
// Usage in vite.config.ts:
//
//   import { generateEnumsPlugin, wasmTypesPlugin } from './vite-plugin-wasm-types'
//
//   export default defineConfig({
//     plugins: [
//       generateEnumsPlugin(["include/algo_types.hpp"]),
//       wasmTypesPlugin(),
//       react(),
//     ]
//   })

import fs from "fs";
import path from "path";
import type { Plugin } from "vite";

// ── shared ────────────────────────────────────────────────────────────────────

const __dirname = path.dirname(new URL(import.meta.url).pathname);

// ── generateEnumsPlugin ───────────────────────────────────────────────────────

interface EnumMember {
  value: number;
  label: string;
}
type ParsedEnum = Record<string, EnumMember>;

function parseEnums(hpp: string): Record<string, ParsedEnum> {
  const enums: Record<string, ParsedEnum> = {};
  const enumRegex = /enum\s+class\s+(\w+)\s*(?::\s*\w+)?\s*\{([^}]+)\}/g;
  let match;
  while ((match = enumRegex.exec(hpp)) !== null) {
    const name = match[1];
    const body = match[2];
    enums[name] = {};
    let counter = 0;
    for (const line of body.split("\n")) {
      const commentMatch = line.match(/\/\/\s*(.+)$/);
      const label = commentMatch?.[1].trim();
      const cleaned = line
        .replace(/\/\/.*$/, "")
        .replace(/,\s*$/, "")
        .trim();
      if (!cleaned) continue;
      const [ident, val] = cleaned.split("=").map((s) => s.trim());
      if (!ident) continue;
      if (ident.endsWith("_COUNT")) continue;
      counter = val !== undefined ? parseInt(val) : counter;
      enums[name][ident] = { value: counter, label: label ?? ident };
      counter++;
    }
  }
  return enums;
}

function generateTs(
  enums: Record<string, ParsedEnum>,
  sourceName: string,
): string {
  const lines = [
    `// AUTO-GENERATED — do not edit. Source: include/${sourceName}`,
    "",
  ];
  for (const [name, members] of Object.entries(enums)) {
    const entries = Object.entries(members);
    lines.push(`export const ${name} = {`);
    for (const [key, { value }] of entries) {
      lines.push(`  ${key}: ${value},`);
    }
    lines.push(`} as const;`);
    lines.push(`export type ${name} = typeof ${name}[keyof typeof ${name}];`);
    lines.push("");
    lines.push(`export const ${name}Label: Record<${name}, string> = {`);
    for (const [, { value, label }] of entries) {
      lines.push(`  [${value}]: "${label}",`);
    }
    lines.push(`};`);
    lines.push("");
  }
  return lines.join("\n");
}

export function generateEnumsPlugin(hppPaths: string[]): Plugin {
  const outDir = path.resolve(__dirname, "../src/types");

  function run(hppPath: string) {
    const sourceName = path.basename(hppPath);
    const outName = sourceName.replace(/\.hpp$/, ".ts");
    const outPath = path.join(outDir, outName);
    const hpp = fs.readFileSync(hppPath, "utf-8");
    const enums = parseEnums(hpp);
    const output = generateTs(enums, sourceName);
    fs.mkdirSync(outDir, { recursive: true });
    fs.writeFileSync(outPath, output);
    console.log(`[generateEnums] src/types/${outName} updated`);
  }

  return {
    name: "generate-enums",
    buildStart() {
      for (const p of hppPaths) run(p);
    },
    configureServer(server) {
      for (const p of hppPaths) server.watcher.add(p);
      server.watcher.on("change", (file) => {
        if (hppPaths.includes(file)) run(file);
      });
    },
  };
}

// ── wasmTypesPlugin ───────────────────────────────────────────────────────────

const HEADER_PATH = "include/debug_panel_fmt.hpp";
const OUTPUT_PATH = "src/types/InfoPanel.ts";

const C_TO_TS: Record<string, string> = {
  uint32_t: "number",
  int32_t: "number",
  uint32: "number",
  int32: "number",
  int: "number",
  float: "number",
  double: "number",
  string: "string",
  bool: "boolean",
};

// Maps annotation tag → interface name suffix
// // @stack          DFS_A  node:int  →  DFS_AStackFrame
// // @AdjacencyMatrix DFS_A node:int  →  DFS_AAdjMatFrame
const TAG_TO_SUFFIX: Record<string, string> = {
  stack: "Stack",
  adjacencymatrix: "AdjMat",
  nodelist: "NodeList",
  nodeeditinfo: "NodeEditInfo",
  scriptstack: "ScriptStack",
  scriptqueue: "ScriptQueue",
};

interface ParsedInterface {
  name: string; // e.g. DFS_AStackFrame
  fields: string; // indented TS field lines
}

function parseAnnotations(src: string): ParsedInterface[] {
  // Match any // @<tag> line, case-insensitive tag
  const tagPattern = /^\/\/\s*@(\w+)\s+(\S+)\s*(.*)/;

  return src.split("\n").flatMap((line): ParsedInterface[] => {
    const m = line.trim().match(tagPattern);
    if (!m) return [];

    const [, tag, algoName, rest] = m;
    const suffix = TAG_TO_SUFFIX[tag.toLowerCase()];
    if (!suffix) return []; // unknown tag — skip

    const fields = rest
      .trim()
      .split(/\s+/)
      .filter(Boolean)
      .map((token) => {
        const [fieldName, cType] = token.split(":");
        // strip [] suffix for array types — just map the base type
        const baseType = C_TO_TS[cType?.replace("[]", "")] ?? "unknown";
        const tsType = cType?.endsWith("[]") ? `${baseType}[]` : baseType;
        return `    ${fieldName}: ${tsType};`;
      })
      .join("\n");

    return [
      {
        name: `${algoName}${suffix}Frame`,
        fields,
      },
    ];
  });
}

function generateInfoPanelTypes(root: string): void {
  const headerAbs = path.resolve(root, HEADER_PATH);
  const outputAbs = path.resolve(root, OUTPUT_PATH);

  if (!fs.existsSync(headerAbs)) {
    console.warn(`[wasm-types] header not found: ${headerAbs}`);
    return;
  }

  const src = fs.readFileSync(headerAbs, "utf8");
  const parsed = parseAnnotations(src);

  if (parsed.length === 0) {
    console.warn(`[wasm-types] no known annotations found in ${HEADER_PATH}`);
    return;
  }

  const interfaces = parsed
    .map(({ name, fields }) => `export interface ${name} {\n${fields}\n}`)
    .join("\n\n");

  const output =
    `// AUTO-GENERATED — do not edit manually\n` +
    `// source: ${HEADER_PATH}\n\n` +
    `${interfaces}\n`;

  fs.mkdirSync(path.dirname(outputAbs), { recursive: true });
  fs.writeFileSync(outputAbs, output, "utf8");
  console.log(`[wasm-types] generated ${OUTPUT_PATH}`);
}

export function wasmTypesPlugin(): Plugin {
  let root: string;

  return {
    name: "vite-plugin-wasm-types",

    configResolved(config) {
      root = config.root;
      generateInfoPanelTypes(root);
    },

    configureServer(server) {
      const headerAbs = path.resolve(root, HEADER_PATH);
      server.watcher.add(headerAbs);
      server.watcher.on("change", (file) => {
        if (file === headerAbs) {
          generateInfoPanelTypes(root);
          server.watcher.emit("change", path.resolve(root, OUTPUT_PATH));
        }
      });
    },

    buildStart() {
      generateInfoPanelTypes(root);
    },
  };
}

