import ipc, { IM } from "../ipc";
import Button from "./components/Button";
import { useEffect, useState } from "react";

function AccountTile({
  username,
  color,
  refresh,
}: {
  username: string;
  color: string;
  refresh: () => void;
}) {
  return (
    <div className="account-tile">
      <div
        className="text"
        style={{ backgroundColor: color }}
        onClick={async () => {
          const password = await ipc.post<string>(
            IM.account_password,
            username
          );

          logoutAcc();
          // login window
          showWindow(5);

          document.querySelector<HTMLInputElement>("#accName")!.value =
            username;
          document.querySelector<HTMLInputElement>("#accPass")!.value =
            password;

          setTimeout(() => {
            loginAcc();
          }, 100);
        }}
      >
        {username}
      </div>
      <div className="buttons">
        {/*<span className="edit material-icons">edit</span>*/}
        <span
          className="delete material-icons"
          onClick={() => {
            ipc.send(IM.account_remove, username);
            refresh();
          }}
        >
          delete
        </span>
      </div>
    </div>
  );
}

interface Account {
  username: string;
  /**
   * Encrypted
   */
  password: string;
  color: string;
  order: number;
}

export default function AccountManager({
  addAccountID,
}: {
  addAccountID: () => number;
}) {
  const [accounts, setAccounts] = useState<null | Account[]>(null);

  useEffect(() => {
    if (accounts === null)
      ipc
        .post<Account[]>(IM.account_list)
        .then((list) => setAccounts(list.sort((a, b) => b.order - a.order)));
  }, [accounts]);

  return accounts ? (
    <>
      {accounts.length ? (
        accounts.map((acc, i) => (
          <AccountTile
            username={acc.username}
            color={acc.color}
            refresh={() => {
              setAccounts(null);
            }}
            key={i}
          />
        ))
      ) : (
        <>No accounts yet.</>
      )}
      <Button
        title="Account"
        text="Add"
        onClick={() => showWindow(addAccountID())}
      />
    </>
  ) : (
    <>Loading...</>
  );
}
