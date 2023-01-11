load("/home/Pyxis/scripts/analytics/analyticsFactory.js");

function alertAnalytics (){
	db.getMongo().setSlaveOk();
	var af = analyticsFactory();
	var gwsses = af.gwssesByName();
	gwsses.forEach(function(gwss) {
		print(gwss.replace(/ /g,'-'));
	});
}

alertAnalytics()
