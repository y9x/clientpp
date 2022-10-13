import ipc, { IM } from "./ipc";
import getConfig from "./menu/useConfig";
import currentSite from "./site";

/**
 * Discord DiscordRichPresence struct
 * void field = 0/nullptr
 */
interface DiscordRichPresence {
  /* CONST CHAR* */ state?: string /* max 128 bytes */;
  /* CONST CHAR* */ details?: string /* max 128 bytes */;
  /* INT64_T */ startTimestamp?: number;
  /* INT64_T */ endTimestamp?: number;
  /* CONST CHAR* */ largeImageKey?: string /* max 32 bytes */;
  /* CONST CHAR* */ largeImageText?: string /* max 128 bytes */;
  /* CONST CHAR* */ smallImageKey?: string /* max 32 bytes */;
  /* CONST CHAR* */ smallImageText?: string /* max 128 bytes */;
  /* CONST CHAR* */ partyId?: string /* max 128 bytes */;
  /* INT */ partySize?: number;
  /* INT */ partyMax?: number;
  /* INT */ partyPrivacy?: number;
  /* CONST CHAR* */ matchSecret?: string /* max 128 bytes */;
  /* CONST CHAR* */ joinSecret?: string /* max 128 bytes */;
  /* CONST CHAR* */ spectateSecret?: string /* max 128 bytes */;
  /* INT8_T */ instance?: number;
}

let last: DiscordRichPresence | undefined;

const equalsLast = (data: DiscordRichPresence | undefined) => {
  if (typeof last === "undefined" && typeof data === "undefined") return true;

  if (typeof last === "undefined" || typeof data === "undefined") return false;

  for (const field in last) {
    if (
      last[field as keyof DiscordRichPresence] !==
      data[field as keyof DiscordRichPresence]
    )
      return false;
  }

  for (const field in data) {
    if (!(field in last)) return false;
  }

  return true;
};

let loadStart = Date.now();
let loaded = false;

const isLoadedData = (data: GameActivityData): data is GameActivityLoadedData =>
  data.id !== null;

const generatePresence = (
  data?: GameActivityData
): DiscordRichPresence | undefined => {
  if (!getConfig().rpc.enabled) return undefined;
  else if (!data || !isLoadedData(data))
    return {
      startTimestamp: loadStart,
      largeImageKey: "icon",
      state: "Loading",
    };

  if (!loaded) {
    loaded = true;
    loadStart = Date.now();
  }

  return {
    startTimestamp: loadStart,
    largeImageKey: "icon",
    state: getConfig().rpc.name ? "In game" : data.map || undefined,
    details: data.mode || undefined,
    largeImageText: (getConfig().rpc.name && data.user) || undefined,
  };
};

const getSomeData = () => {
  try {
    return getGameActivity();
  } catch (err) {
    // undefined function
  }
};

export const updateRPC = () => {
  const presence = generatePresence(getSomeData());

  if (!equalsLast(presence)) {
    last = presence;
    ipc.send(IM.rpc_update, presence);
  }
};

if (currentSite === "game") {
  updateRPC();
  setInterval(() => updateRPC(), 2000);
}
