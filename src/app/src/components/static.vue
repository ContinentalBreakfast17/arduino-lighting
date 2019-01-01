<template>
	<div>
		<input type="color" id="html5colorpicker" @change="hex" v-model="hexVal" ref="hex" style="width: 100%; margin-bottom:1rem;" class="rgb">
		<RgbInput channel="R" :val="profile.color[0]" @change="setChannel(0, $event)" :gradient="gradients.R" class="rgb" ref="r"/>
		<RgbInput channel="G" :val="profile.color[1]" @change="setChannel(1, $event)" :gradient="gradients.G" class="rgb" ref="g"/>
		<RgbInput channel="B" :val="profile.color[2]" @change="setChannel(2, $event)" :gradient="gradients.B" class="rgb" ref="b"/>

		<select v-model="profile.mode">
			<option value="0">Static</option>
			<option value="1">Rainbow</option>
			<option value="2">Fade</option>
			<option value="4">Strobe</option>
		</select>

		<!--add tooltip to Transition delay -->
		<span v-if='profile.mode != "0"'> Transition delay: <input type="number" v-model="profile.wait" min="5" max="2000" step="1"> ms</span>

	</div>
</template>

<script>
	import RgbInput from './rgb.vue';

	export default {
		props: {
			profile: Object
		},

		data: function() {
			return {
				hexVal: "#000000",
				gradients: {
					R: ["#000000", "#ff0000"],
					G: ["#000000", "#00ff00"],
					B: ["#000000", "#0000ff"]
				}
			}
		},

		components: {
			RgbInput
		},

		mounted: function(){
			this.fix();
		},

		methods: {
			fix: function() {
				this.fixChannels();
				this.fixGradients();
				this.fixHex();
			},
			fixChannels: function() {
				this.$refs.r.setValue(this.profile.color[0]);
				this.$refs.g.setValue(this.profile.color[1]);
				this.$refs.b.setValue(this.profile.color[2]);
			},
			fixGradients: function() {
				var color = this.profile.color;
				this.gradients.R = [rgbToHex([0, color[1], color[2]]), rgbToHex([255, color[1], color[2]])];
				this.gradients.G = [rgbToHex([color[0], 0, color[2]]), rgbToHex([color[0], 255, color[2]])];
				this.gradients.B = [rgbToHex([color[0], color[1], 0]), rgbToHex([color[0], color[1], 255])];
			},
			fixHex: function() {
				this.hexVal = rgbToHex(this.profile.color);
			},
			hex: function() {
				this.hexVal = this.$refs.hex.value;
				var color = parseColor(this.hexVal);
				this.profile.color = color.slice();
				this.fix();
			},
			setChannel: function(channel, val) {
				this.profile.color[channel] = parseInt(val, 10);
				this.fix();
			}
		}
	}


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
</script>

<style>

	.rgb {
		margin: 2rem 0;
	}

</style>