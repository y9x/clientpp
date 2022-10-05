declare interface SettingsTab {
  name: string;
  categories: [];
}

declare interface GameWindow {
  // required
  header: string;
  label: string;
  html: "";
  gen(): string;
  // optional
  tabIndex?: number;
  width?: number;
  popup?: boolean;
  sticky?: boolean;
  dark?: boolean;
  hideScroll?: boolean;
}

declare interface Settings extends GameWindow {
  genList(): string;
  getSettings(): string;
  /**
   * Record<settingType: string, GameWindowTab[]>
   */
  tabs: Record<string, SettingsTab[]>;
  settingType: string;
  tabIndex: number;
}

declare const windows: GameWindow[];

/**
 *
 * @param id ID of the window starting from 1.
 */
declare function showWindow(id: number): void;

declare const SOUND: {
  play(id: string, vol: number): void;
};

declare function playTick(): void;
