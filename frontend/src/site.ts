const paths: Record<string, "game" | "social" | "editor" | undefined> = {
  "/": "game",
  "/social.html": "social",
  "/editor.html": "editor",
};

const currentSite =
  global.location.hostname === "docs.krunker.io"
    ? "docs"
    : paths[global.location.pathname];

export default currentSite;
