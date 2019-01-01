<template>

	<div id="app">

		<vue-tabs ref="tabs" @tab-change="handleTabChange">
			
			<v-tab v-for="profile in profiles" :key="profile.id" :title="profile.name">
				<div slot="title"><span>{{ profile.name }} <button @click="deleteProfile(profile.id)">x</button></span></div>

				<div style="width: 80%; margin: auto;">
					<Static :profile="profile.sttc"/>
				</div>
			</v-tab>
			<v-tab title="+"/>

		</vue-tabs>

	</div>

</template>

<script>
import Static from './components/static.vue'

export default {
	name: 'app',
	components: {
		Static
	},

	methods: {
		handleTabChange: function(tabIndex, newTab){
			if(newTab.title == "+") {
				this.addProfile()
			}
		},

		addProfile: function(){
			var id = uuidv4()
			var profile = {
				id: id,
				name: "Profile " + (this.profiles.length + 1).toString(),
				sttc: {
					color: [0, 0, 0],
					pins: [0, 0, 0],
					mode: 0,
					wait: 5
				},
				addr: {}
			};
			this.profiles.push(profile);
		},

		deleteProfile: function(id) {
			var index = this.profiles.findIndex((e) => {
				return e.id === id;
			});
			this.profiles.splice(index, 1);
		}
	},

	data: function() {
		return {
			profiles: [
				{
					id: "1",
					name: "Red",
					sttc: {
						color: [255, 0, 0],
						pins: [9, 10, 11],
						mode: 0,
						wait: 5
					},
					addr: {}
				},
				{
					id: "2",
					name: "Green",
					sttc: {
						color: [0, 255, 0],
						pins: [9, 10, 11],
						mode: 0,
						wait: 5
					},
					addr: {}
				},
				{
					id: "3",
					name: "Blue",
					sttc: {
						color: [0, 0, 255],
						pins: [9, 10, 11],
						mode: 0,
						wait: 5
					},
					addr: {}
				}
			]
		}
	}
}

function uuidv4() {
	return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, function(c) {
		var r = Math.random() * 16 | 0, v = c == 'x' ? r : (r & 0x3 | 0x8);
		return v.toString(16);
	});
}

</script>

<style>
#app {
	font-family: 'Avenir', Helvetica, Arial, sans-serif;
	-webkit-font-smoothing: antialiased;
	-moz-osx-font-smoothing: grayscale;
	text-align: center;
	color: #2c3e50;
	margin-top: 60px;
}
</style>
