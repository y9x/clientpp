const paths = {
  "/": "game",
  "/social.html": "social",
  "/editor.html": "editor",
};

/**
 * @type {"game"|"social"|"editor"|undefined}
 */
const currentSite = paths[location.pathname];

export default currentSite;
