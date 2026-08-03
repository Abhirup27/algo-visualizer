
#ifdef PLATFORM_WEB
#include "utils.hpp"
#include "web.hpp"
#include <stdlib.h>
// Toggle console visibility

extern "C" {
EM_JS(void, set_mode, (int major, int minor),
      { window.setMode(major, minor); });

EM_JS(void, js_register_algorithms, (const char *json), {
  const obj = JSON.parse(UTF8ToString(json));
  window.dispatchEvent(new CustomEvent("register_algorithms", {detail : obj}));
});

EM_JS(void, close_window, (void), { window.close(); });
// Print a float value
EM_JS(void, print_float, (float val), { Module.print(val); });

// Print a UTF-8 string
EM_JS(void, print_string, (const char *str),
      { Module.print(UTF8ToString(str)); });

void print_console(const char *str) { print_string(str); };
void print_console_float(float val) { print_float(val); };
void onPreloadSuccess(const char *filename) {
  TRACELOG(LOG_INFO, "File loaded: %s", filename);
}

void onPreloadError(const char *fileName) {
  TRACELOG(LOG_WARNING, "File failed to load: %s", fileName);
}
#include <stdlib.h> // For free()

EM_JS(char *, list_all_files, (const char *dir), {
  const currentDir = UTF8ToString(dir);
  let output = 'Files in ' + currentDir + ':\n';

  function traverse(folder) {
    const names = FS.readdir(folder);
    for (let i = 0; i < names.length; i++) {
      const name = names[i];
      if (name == '.' || name == '..')
        continue; // Fixed: removed spaces in ===

      const path = folder + '/' + name;
      try {
        const node = FS.lookupPath(path).node;
        if (FS.isFile(node.mode)) {
          output += '[FILE] ' + path + '\n';
        } else if (FS.isDir(node.mode)) {
          output += '[DIR]  ' + path + '\n';
          traverse(path); // Recursive call for subdirectories
        }
      } catch (e) {
        output += '[ERROR] ' + path + ': ' + e.message + '\n';
      }
    }
  }

  try {
    traverse(currentDir);
  } catch (e) {
    output = 'Error reading directory: ' + e.message;
  }

  // Return the string allocated on the Emscripten heap
  return stringToNewUTF8(output);
});
}
void list_files() {
  char *fileList = list_all_files("/");
  if (fileList) {
    print_console(fileList);
    free(fileList);
  } else {
    print_console("Failed to list files\n");
  }
}

#endif // PLATFORM_WEB

