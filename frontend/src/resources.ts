/* eslint-disable no-new-func */
import console from "./console";
import ipc, { IM, ipcConsole } from "./ipc";
import { css, js } from "./runtime";
// import evalLegacyUserscript from "./legacyUserscript";
// import evalChiefUserscript from "./chiefUserscript";
import currentSite from "./site";

const add_css = () => {
  for (const [, data] of css) {
    const url = URL.createObjectURL(new Blob([data], { type: "text/css" }));

    const link = document.head.appendChild(
      Object.assign(document.createElement("link"), {
        rel: "stylesheet",
        href: url,
      })
    );

    link.addEventListener("load", () => {
      URL.revokeObjectURL(url);
    });

    document.head.appendChild(link);
  }
};

new MutationObserver((mutations, observer) => {
  for (const mutation of mutations) {
    for (const node of mutation.addedNodes) {
      if (
        node instanceof HTMLLinkElement &&
        new URL(node.href || "/", global.location.toString()).pathname ===
          "/css/main_custom.css"
      ) {
        add_css();
        observer.disconnect();
      }
    }
  }
}).observe(document, {
  childList: true,
  subtree: true,
});

type ResourceType =
  | "all"
  | "document"
  | "stylesheet"
  | "image"
  | "media"
  | "font"
  | "x"
  | "fetch"
  | "websocket"
  | "manifest"
  | "signed"
  | "ping"
  | "other"
  | "unknown";

const urlFilters: [
  script: string,
  filter: Required<ExportedUserscriptData>["filterURL"]
][] = [];

ipc.on(IM.will_block_url, (id: number, url: string, type: ResourceType) => {
  for (const [script, filter] of urlFilters)
    if (filter(new URL(url), type)) {
      ipcConsole.debug(`${script} blocked ${type}: ${url}`);
      return ipc.send(id, true);
    }

  ipc.send(id, false);
});

interface ExportedUserscriptData {
  /**
   * Core
   */
  main?: (data: UserscriptRuntime) => void;
  /*
   * Feature - Filters an incoming network request.
   */
  filterURL?: (url: URL, resourceType: ResourceType) => boolean;
}

type ExportUserscriptCallback = (data: ExportedUserscriptData) => void;

type UserscriptConsole = typeof console;

interface UserscriptRuntime {
  getSite: () => typeof currentSite;
}

for (const [script, code] of js) {
  const run = new Function("console", "exportUserscript", code) as (
    console: UserscriptConsole,
    exportUserscript: ExportUserscriptCallback
  ) => never;

  run(console, (data) => {
    if (data.filterURL) urlFilters.push([script, data.filterURL]);

    if (data.main)
      data.main({
        getSite: () => currentSite,
      });
  });

  console.log("GOT SCRIPT", script, "....");

  // if (metadata) evalChiefUserscript(name, metadata, data);
  // else evalLegacyUserscript(name, data);
}
