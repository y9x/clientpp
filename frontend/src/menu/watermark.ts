import { clientVersion } from "../runtime";

const createWatermark = () => {
  const watermark = document.createElement("div");
  watermark.id = "watermark";
  watermark.style.position = "absolute";
  watermark.style.color = "rgba(0,0,0, 0.3)";
  watermark.style.bottom = "0";
  watermark.style.left = "20px";
  watermark.style.fontSize = "6pt";
  watermark.innerHTML = "Chief Client v" + clientVersion;
  document.querySelector("#gameUI")?.append(watermark);
};

export default createWatermark;
