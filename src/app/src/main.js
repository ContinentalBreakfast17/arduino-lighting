import Vue from 'vue'
import App from './App.vue'

import VueTabs from 'vue-nav-tabs';
import 'vue-nav-tabs/themes/vue-tabs.css';

Vue.config.productionTip = false

Vue.use(VueTabs);

new Vue({
  render: h => h(App),
}).$mount('#app')
