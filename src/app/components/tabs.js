var tabs = new Vue({
	el: "#tabs",

	data: function() {
		return {
			profiles: ["Profile 1", "Profile 2"]
		}
	},

	methods: {
		removeTab: function(index){
			if(this.profiles.length == 1) return;
			this.profiles.splice(index, 1);
		},
		addTab: function() {
			this.profiles.push("Profile " + profiles.length+1);
		}
	}
})