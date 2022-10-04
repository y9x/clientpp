const metadata = {
	"name": "Sky Color",
	"author": "Chief Software",
	"description": "Adds a sky color picker to your client's settings.",
	"version": 0.2,
	"locations": ["game"],
	"features": {
		"gui": {
			"Sky Color": {
				"Enabled": {
					"type": "boolean",
					"walk": "skycolor.enabled",
					"change": "skycolor_change"
				},
				"Color": {
					"type": "color",
					"walk": "skycolor.color",
					"change": "skycolor_change"
				}
			}
		},
		"config": {
			"skycolor": {
				"enabled": true,
				"color": "#64d9a8"
			}
		}
	}
};

let color;
const { config } = metadata.features;

Object.defineProperty(Object.prototype, "skyCol", {
	get(){
		if (config.skycolor.enabled)
			return config.skycolor.color;
	},
	set(value){
		color = value;
	},
});

export function skycolor_change(value, init){
	if (!init) location.reload();
}