import console from "./console";
import ipc, { IM, ipcConsole } from "./ipc";
import ButtonControl from "./menu/components/ButtonControl";
import ColorControl from "./menu/components/ColorControl";
import Control from "./menu/components/Control";
import FileControl from "./menu/components/FileControl";
import LinkControl from "./menu/components/LinkControl";
import SelectControl from "./menu/components/SelectControl";
import { Set, HeadlessSet } from "./menu/components/Set";
import SliderControl from "./menu/components/SliderControl";
import SwitchControl from "./menu/components/SwitchControl";
import TextControl from "./menu/components/TextControl";
import { css, js } from "./runtime";
import currentSite from "./site";
import useLocalStorage from "./useLocalStorage";
import htm from "htm";
import React from "react";

const UserscriptUI = Object.freeze({
  ButtonControl,
  ColorControl,
  Control,
  FileControl,
  LinkControl,
  SelectControl,
  Set,
  HeadlessSet,
  SliderControl,
  SwitchControl,
  TextControl,
});

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

type UserscriptContext = (
  code: string,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  React: typeof import("react"),
  // expose `htm` to allow for manipulating JSX
  html: typeof htm,
  // important hook for lightweight configs
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  useLocalStorage: typeof import("./useLocalStorage").default,
  // expose components for building a GUI extension
  UI: typeof UserscriptUI,
  // eslint-disable-next-line @typescript-eslint/consistent-type-imports
  console: typeof import("./console").default,
  exportUserscript: ExportUserscriptCallback
) => void;

interface UserscriptRuntime {
  getSite: () => typeof currentSite;
}

for (const [script, code] of js) {
  // eslint-disable-next-line no-new-func
  const run = new Function(
    "code",
    "React",
    "html",
    "useLocalStorage",
    "UI",
    "console",
    "exportUserscript",
    "eval(code)"
  ) as UserscriptContext;

  run(
    code + "//# sourceURL=" + new URL("file:" + script).toString(),
    React,
    htm.bind(React.createElement),
    useLocalStorage,
    UserscriptUI,
    console,
    (data) => {
      if (data.filterURL) urlFilters.push([script, data.filterURL]);

      if (data.main)
        data.main({
          getSite: () => currentSite,
        });
    }
  );

  console.log("GOT SCRIPT", script, "....");

  // if (metadata) evalChiefUserscript(name, metadata, data);
  // else evalLegacyUserscript(name, data);
}
