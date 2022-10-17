import { waitFor } from "../utils";
import AccountManager from "./AccountManager";
import AddAccount from "./AddAccount";
import Menu from "./Menu";
import extendRoot from "./extendSettings";
import extendWindows from "./extendWindows";
import getConfig from "./useConfig";
import createWatermark from "./watermark";

// settings = 0
extendRoot("Client", () => <Menu />);

document.addEventListener("DOMContentLoaded", () => {
  if (getConfig().client.watermark) createWatermark();
});

if (getConfig().game.account_manager.enabled)
  waitFor(
    () => typeof windows === "object" && Array.isArray(windows) && windows
  ).then(() => {
    const addAccountID = extendWindows(
      {
        header: "Account Manager",
        label: "account_manager",
        width: 1100,
        popup: true,
      },
      () => <AddAccount accountManagerID={() => accountManagerID} />
    );

    const accountManagerID = extendWindows(
      {
        header: "Account Manager",
        label: "account_manager",
        width: 1100,
        popup: true,
      },
      () => <AccountManager addAccountID={() => addAccountID} />
    );

    const createButton = (bar: HTMLDivElement) => {
      bar.innerHTML += `<div class="button buttonG lgn" style="width: 200px; margin-right: 0px; padding-top: 5px; margin-left: 5px; padding-bottom: 13px;" onmouseenter="playTick()" onclick="showWindow(${accountManagerID})">Accounts <span class="material-icons" style="vertical-align: middle; color: rgb(255, 255, 255); font-size: 36px; margin-top: -8px;">switch_account</span></div>`;
    };

    waitFor(() =>
      document.querySelector<HTMLDivElement>("#signedInHeaderBar")
    ).then(createButton);

    waitFor(() =>
      document.querySelector<HTMLDivElement>("#signedOutHeaderBar")
    ).then(createButton);
  });
