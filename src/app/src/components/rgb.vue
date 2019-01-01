<template>
	<span class="rgbContainer">
		<span class="channelLabel">{{ channel }}:</span>
		<input type="number" v-model="cpy" @change="$emit('change', cpy)" min="0" max="255" class="channelTextbox">
		<input type="range" v-model="cpy" @change="$emit('change', cpy)" min="0" max="255" class="slider" :style="{ 'background-image' : 'linear-gradient(to right, ' + gradient[0] + ', ' + gradient[1] +')' }"> 
	</span>
</template>

<script>
	export default {
		props: {
			channel:  String,
			val:      Number,
			gradient: Array,
		},

		data: function() {
			return {
				cpy: Number
			}
		},

		created: function(){
			this.setValue(this.val);
		},

		methods: {
			setValue: function(val){
				this.cpy = val;
			}
		}
	}
</script>

<style>
	.rgbContainer {
		display: flex;
		flex-direction: row;
		flex-wrap: nowrap;
		justify-content: center;
		align-items: center;
	}

	.slider {
		-webkit-appearance: none;
		flex-grow: 1;
		order: 2;
		height: 15px;
		border-radius: 5px;   
		outline: none;
		opacity: 1;
		-webkit-transition: .2s;
		transition: opacity .2s;
	}

	.slider::-webkit-slider-thumb {
		-webkit-appearance: none;
		appearance: none;
		width: 25px;
		height: 25px;
		border-radius: 50%; 
		background: #404040;
		cursor: pointer;
	}

	.slider::-moz-range-thumb {
		width: 25px;
		height: 25px;
		border-radius: 50%;
		background: #404040;
		cursor: pointer;
	}

	.channelLabel {
		width: 20px;
		order: 1;
	}

	.channelTextbox {
		width: 50px;
		order: 3;
	}
</style>