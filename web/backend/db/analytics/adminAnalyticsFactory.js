load("/home/Pyxis/scripts/analytics/uuidhelpers.js");

function adminAnalyticsFactory() {
	return {
		replSetStatus: function() {
			return db.runCommand({replSetGetStatus:1});
		},
		dbStats: function() {
			return db.stats();
		}
	}
}

