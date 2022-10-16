import ipc, { IM } from "../ipc";
import { useRef } from "react";

export default function AddAccount({
  accountManagerID,
}: {
  accountManagerID: () => number;
}) {
  const username = useRef<HTMLInputElement | null>(null);
  const password = useRef<HTMLInputElement | null>(null);
  const color = useRef<HTMLInputElement | null>(null);

  return (
    <>
      <div id="referralHeader">Add Account</div>
      <div style={{ height: 20 }}></div>
      <input
        id="accName"
        type="text"
        ref={username}
        placeholder="Enter Username"
        className="accountInput"
        style={{ marginTop: 0 }}
      />
      <input
        id="accPass"
        type="password"
        ref={password}
        placeholder="Enter Password"
        className="accountInput"
      />
      <input id="accColor" type="color" defaultValue="#2196f3" ref={color} />
      <div
        style={{
          width: "100%",
          textAlign: "center",
          marginTop: 10,
          backgroundColor: "rgba(0,0,0,0.3)",
          paddingTop: 10,
          paddingBottom: 20,
        }}
      >
        <div
          className="accBtn button buttonG"
          style={{ width: "calc(100% - 30px)" }}
          onClick={() => {
            ipc.send(
              IM.account_set_password,
              username.current!.value,
              password.current!.value,
              color.current!.value,
              0
            );

            showWindow(accountManagerID());
          }}
        >
          Add
        </div>
      </div>
    </>
  );
}
