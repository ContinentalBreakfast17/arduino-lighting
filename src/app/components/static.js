var staticConfig = new Vue({
	el: "#static",

	data: function() {
		return {
			hexVal: "#000000",
			gradients: {
				R: ["#000000", "#ff0000"],
				G: ["#000000", "#00ff00"],
				B: ["#000000", "#0000ff"]
			},
			profile: {
				mode: 0,
				wait: 5,
				color: [0,0,0]
			}
		}
	},

	components: {
		'rgb-input': {
			props: {
				channel:  String,
				val:      Number,
				gradient: Array,
			},
			template: `
				<span class="rgbContainer">
					<h class="channelLabel">{{ channel }}:</h>
					<input type="number" v-model="val" v-on:change="$emit('color-change', val)" min="0" max="255" class="channelTextbox"></input>
					<input type="range" v-model="val" v-on:change="$emit('color-change', val)" min="0" max="255" class="slider" v-bind:style="{ 'background-image' : 'linear-gradient(to right, ' + gradient[0] + ', ' + gradient[1] +')' }"></input> 
				</span>
			`
		}
	},

	methods: {
		fixGradients: function() {
			color = this.profile.color;
			this.gradients.R = [rgbToHex([0, color[1], color[2]]), rgbToHex([255, color[1], color[2]])];
			this.gradients.G = [rgbToHex([color[0], 0, color[2]]), rgbToHex([color[0], 255, color[2]])];
			this.gradients.B = [rgbToHex([color[0], color[1], 0]), rgbToHex([color[0], color[1], 255])];
		},
		hex: function() {
			this.hexVal = document.getElementById("hexColor").value;
			var color = parseColor(this.hexVal);
			this.profile.color = color.slice();
			this.fixGradients();
		},
		setChannel: function(channel, val) {
			this.profile.color[channel] = parseInt(val, 10);
			this.hexVal = rgbToHex(this.profile.color);
			this.fixGradients();
		},
	}
})


function parseColor(input) {
	var m = input.match(/^#([0-9a-f]{6})$/i)[1];
	if(m) {
		return [
			parseInt(m.substr(0,2),16),
			parseInt(m.substr(2,2),16),
			parseInt(m.substr(4,2),16)
		];
	}
}

function componentToHex(c) {
	if(typeof c === "string") {
		c = parseInt(c, 10);
	}
	var hex = c.toString(16);
	return hex.length == 1 ? "0" + hex : hex;
}

function rgbToHex(color) {
	return "#" + componentToHex(color[0]) + componentToHex(color[1]) + componentToHex(color[2]);
}