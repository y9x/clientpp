import currentSite from "../site";
import extendRoot from "./extendSettings";
import Menu from "./Menu";

if (currentSite === "game") {
  // settings = 0
  extendRoot("Client").then((root) => {
    root.render(<Menu />);
  });
}
