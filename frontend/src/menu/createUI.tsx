import currentSite from "../site";
import { wait_for } from "../utils";
import LinkControl from "./components/LinkControl";
import extendRoot from "./extendSettings";
import extendWindows from "./extendWindows";
import Menu from "./Menu";

if (currentSite === "game") {
  // settings = 0
  extendRoot("Client", (root) => root.render(<Menu />));

  extendWindows("Account Manager", "account_manager", (root) =>
    root.render(
      <LinkControl title="Test" href="https://google.com/"></LinkControl>
    )
  ).then((id) => {
    const createButton = (bar: HTMLDivElement) => {
      bar.innerHTML += `<div class="button buttonG lgn" style="width: 200px; margin-right: 0px; padding-top: 5px; margin-left: 5px; padding-bottom: 13px;" onmouseenter="playTick()" onclick="showWindow(${
        id + 1
      })">Accounts <span class="material-icons" style="vertical-align: middle; color: rgb(255, 255, 255); font-size: 36px; margin-top: -8px;">switch_account</span></div>`;
    };

    wait_for(() =>
      document.querySelector<HTMLDivElement>("#signedInHeaderBar")
    ).then(createButton);

    wait_for(() =>
      document.querySelector<HTMLDivElement>("#signedOutHeaderBar")
    ).then(createButton);

    console.log("account_manager:", id);
  });
}
